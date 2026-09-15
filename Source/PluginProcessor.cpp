#include "PluginProcessor.h"
#include "PluginEditor.h"

KickBassSpaceAudioProcessor::KickBassSpaceAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
                        .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output",    juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    setLatencySamples (0);
}

KickBassSpaceAudioProcessor::~KickBassSpaceAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout KickBassSpaceAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "attackMs", "Attack", juce::NormalisableRange<float> (0.1f, 50.0f, 0.01f, 0.4f), 3.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "releaseMs", "Release", juce::NormalisableRange<float> (10.0f, 1000.0f, 0.1f, 0.4f), 150.0f));

    return { params.begin(), params.end() };
}

void KickBassSpaceAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);
    currentSampleRate = sampleRate;

    kickOverallEnvelope.prepare (sampleRate);
    bassOverallEnvelope.prepare (sampleRate);
    kickAnalyzer.prepare (sampleRate);
    bassAnalyzer.prepare (sampleRate);

    samplesPerHistoryPoint = juce::jmax (1, (int) (sampleRate / historyRateHz));
    samplesUntilNextHistoryPoint = samplesPerHistoryPoint;

    const juce::SpinLock::ScopedLockType lock (snapshotLock);
    snapshot = EnvelopeSnapshot {};
}

void KickBassSpaceAudioProcessor::releaseResources() {}

bool KickBassSpaceAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto mainIn = layouts.getChannelSet (true, 0);
    auto mainOut = layouts.getChannelSet (false, 0);

    if (mainIn != juce::AudioChannelSet::mono() && mainIn != juce::AudioChannelSet::stereo())
        return false;
    if (mainIn != mainOut)
        return false;

    auto sideIn = layouts.getChannelSet (true, 1);
    if (! sideIn.isDisabled() && sideIn != juce::AudioChannelSet::mono() && sideIn != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void KickBassSpaceAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    auto mainBlock = getBusBuffer (buffer, true, 0);
    auto sideBlock = getBusBuffer (buffer, true, 1);

    auto numSamples = mainBlock.getNumSamples();
    auto numMainChannels = mainBlock.getNumChannels();
    auto numSideChannels = sideBlock.getNumChannels();

    float attackMs = apvts.getRawParameterValue ("attackMs")->load();
    float releaseMs = apvts.getRawParameterValue ("releaseMs")->load();
    kickOverallEnvelope.setAttackTimeMs (attackMs);
    kickOverallEnvelope.setReleaseTimeMs (releaseMs);
    bassOverallEnvelope.setAttackTimeMs (attackMs);
    bassOverallEnvelope.setReleaseTimeMs (releaseMs);
    kickAnalyzer.setAttackReleaseMs (attackMs, releaseMs);
    bassAnalyzer.setAttackReleaseMs (attackMs, releaseMs);

    constexpr float conflictThreshold = 0.02f;

    float sidechainActivityThisBlock = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float kickSample = 0.0f;
        for (int ch = 0; ch < numMainChannels; ++ch)
            kickSample += mainBlock.getSample (ch, i);
        if (numMainChannels > 0) kickSample /= (float) numMainChannels;

        float bassSample = 0.0f;
        for (int ch = 0; ch < numSideChannels; ++ch)
            bassSample += sideBlock.getSample (ch, i);
        if (numSideChannels > 0) bassSample /= (float) numSideChannels;

        sidechainActivityThisBlock = juce::jmax (sidechainActivityThisBlock, std::abs (bassSample));

        auto kickEnvOverall = kickOverallEnvelope.processSample (kickSample);
        auto bassEnvOverall = bassOverallEnvelope.processSample (bassSample);

        auto kickBands = kickAnalyzer.processSample (kickSample);
        auto bassBands = bassAnalyzer.processSample (bassSample);

        std::array<float, numBands> conflictBands {};
        float maxConflict = 0.0f;
        for (int b = 0; b < numBands; ++b)
        {
            auto c = juce::jmin (kickBands[(size_t) b], bassBands[(size_t) b]);
            if (c < conflictThreshold) c = 0.0f;
            conflictBands[(size_t) b] = c;
            maxConflict = juce::jmax (maxConflict, c);
        }

        if (--samplesUntilNextHistoryPoint <= 0)
        {
            samplesUntilNextHistoryPoint = samplesPerHistoryPoint;

            const juce::SpinLock::ScopedLockType lock (snapshotLock);
            snapshot.kick[(size_t) snapshot.writePos] = kickEnvOverall;
            snapshot.bass[(size_t) snapshot.writePos] = bassEnvOverall;
            snapshot.conflict[(size_t) snapshot.writePos] = maxConflict;
            snapshot.conflictPerBand[(size_t) snapshot.writePos] = conflictBands;
            snapshot.writePos = (snapshot.writePos + 1) % historySize;
        }
    }

    sidechainActivitySmoothed = juce::jmax (sidechainActivitySmoothed * 0.999f, sidechainActivityThisBlock);
    {
        const juce::SpinLock::ScopedLockType lock (snapshotLock);
        snapshot.sidechainConnected = (numSideChannels > 0) && (sidechainActivitySmoothed > 0.0001f);
    }

    juce::ignoreUnused (buffer);
}

void KickBassSpaceAudioProcessor::getEnvelopeSnapshot (EnvelopeSnapshot& outSnapshot) const
{
    const juce::SpinLock::ScopedLockType lock (snapshotLock);
    outSnapshot = snapshot;
}

juce::AudioProcessorEditor* KickBassSpaceAudioProcessor::createEditor()
{
    return new KickBassSpaceAudioProcessorEditor (*this);
}

void KickBassSpaceAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void KickBassSpaceAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KickBassSpaceAudioProcessor();
}
