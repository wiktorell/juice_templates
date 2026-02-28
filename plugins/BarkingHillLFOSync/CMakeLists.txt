cmake_minimum_required(VERSION 3.22)

# ──────────────────────────────────────────────────────────────────────────────
# BarkingHillLFOSync — CMakeLists.txt
# Stage 1: Foundation + Shell
# ──────────────────────────────────────────────────────────────────────────────
# NOTE: No project() or add_subdirectory(JUCE) here.
# JUCE is added at root level (parent CMakeLists.txt).
# ──────────────────────────────────────────────────────────────────────────────

juce_add_plugin(BarkingHillLFOSync
    COMPANY_NAME             "BarkingHill"
    PLUGIN_MANUFACTURER_CODE Bkhl
    PLUGIN_CODE              BhLS
    FORMATS                  VST3 AU Standalone
    PRODUCT_NAME             "LFO Sync"
    IS_SYNTH                 FALSE
    IS_MIDI_EFFECT           TRUE
    NEEDS_MIDI_INPUT         TRUE
    NEEDS_MIDI_OUTPUT        TRUE
    NEEDS_WEB_BROWSER        TRUE   # Required for VST3 WebView support (pattern #9)
)

# ── Source files ──────────────────────────────────────────────────────────────
target_sources(BarkingHillLFOSync
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

# ── Include paths ─────────────────────────────────────────────────────────────
target_include_directories(BarkingHillLFOSync
    PRIVATE
        Source
)

# ── JUCE module dependencies ──────────────────────────────────────────────────
target_link_libraries(BarkingHillLFOSync
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra             # Required for WebBrowserComponent (pattern #3)
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ── Generate JuceHeader.h — MUST be after target_link_libraries (pattern #1) ─
juce_generate_juce_header(BarkingHillLFOSync)

# ── WebView UI binary data ────────────────────────────────────────────────────
juce_add_binary_data(BarkingHillLFOSync_UIResources
    SOURCES
        Source/ui/public/index.html
        Source/ui/public/js/juce/index.js
        Source/ui/public/js/juce/check_native_interop.js
)

target_link_libraries(BarkingHillLFOSync
    PRIVATE
        BarkingHillLFOSync_UIResources
)

# ── Compile definitions ───────────────────────────────────────────────────────
target_compile_definitions(BarkingHillLFOSync
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=1              # Enable WebBrowserComponent (pattern #3)
        JUCE_USE_CURL=0                 # No CURL needed for local HTML
)
