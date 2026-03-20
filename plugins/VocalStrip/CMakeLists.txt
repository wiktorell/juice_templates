cmake_minimum_required(VERSION 3.15)

juce_add_plugin(VocalStrip
    COMPANY_NAME "Barking Hill"
    PLUGIN_MANUFACTURER_CODE Brkh
    PLUGIN_CODE Vstr
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "VocalStrip"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    NEEDS_WEB_BROWSER TRUE
)

target_sources(VocalStrip
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_include_directories(VocalStrip
    PRIVATE
        Source
)

target_link_libraries(VocalStrip
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# CRITICAL: Generate JuceHeader.h AFTER target_link_libraries (JUCE 8 requirement)
juce_generate_juce_header(VocalStrip)

# WebView UI Resources
juce_add_binary_data(VocalStrip_UIResources
    SOURCES
        Source/ui/public/index.html
        Source/ui/public/js/juce/index.js
        Source/ui/public/js/juce/check_native_interop.js
)

target_link_libraries(VocalStrip
    PRIVATE
        VocalStrip_UIResources
)

target_compile_definitions(VocalStrip
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
)
