#include "config.h"
#include "mods/api.h"
#include "mods/svc/config.h"
#include "mods/svc/log.hpp"
#include "mods/svc/ui.h"
#include "types.h"

static UiWindowHandle controlsWindowHandle;
static ConfigVarHandle wolfLinkSwimmingHandle;
static ConfigVarHandle maxOxygenHandle;
static ConfigVarHandle useYHandle;
static ConfigVarHandle keepCameraOnDeathHandle;
static ConfigVarHandle fasterGameOverHandle;

static void addControl(UiElementHandle pane, const UiControlDesc& desc) {
    svc_ui->pane_add_control(mod_ctx, pane, &desc, nullptr);
}

static UiControlDesc createConfigControl(ConfigVarHandle config_var, UiControlKind kind,
    const char* label, const char* desc)
{
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.binding = UI_BINDING_CONFIG_VAR;
    control.config_var = config_var;
    control.kind = kind;
    control.label = label;
    control.help_rml = desc;

    return control;
}

static ModResult buildSettingsTab(
    ModContext*, UiWindowHandle, UiElementHandle left, UiElementHandle right, 
    void*, ModError*) {

    UiControlDesc control;

    svc_ui->pane_add_section(mod_ctx, left, "Gameplay");

    control = createConfigControl(wolfLinkSwimmingHandle, UI_CONTROL_TOGGLE,
        "Enable Wolf Link Diving",
        "Allows you to dive underwater as Wolf Link."
        "<br/><br/>"
        "By default, press B to swim down, and A to swim up."
    );
    addControl(left, control);

    control = createConfigControl(useYHandle, UI_CONTROL_TOGGLE,
        "Use Y to Swim Up",
        "Press Y instead of A to swim up as Wolf Link."
        "<br/><br/>"
        "Use this if you're used to the old code, or if you run into problems "
        "with the default setting."
    );
    control.is_disabled = [](ModContext*, void*) { 
        return !Config::getWolfLinkSwimming(); 
    };
    addControl(left, control);

    control = createConfigControl(maxOxygenHandle, UI_CONTROL_NUMBER,
        "Max Oxygen (seconds)",
        "Changes how long you can last underwater, in seconds."
        "<br/><br/>"
        "Vanilla default is 20 seconds."
    );
    control.min = 0;
    control.max = 71582788; // max to prevent overflowing in calcs
    addControl(left, control);

    control = createConfigControl(keepCameraOnDeathHandle, UI_CONTROL_TOGGLE,
        "Don't Change Camera On Death",
        "Prevents the camera from changing to the cinematic view "
        "upon dying."
    );
    addControl(left, control);

    control = createConfigControl(fasterGameOverHandle, UI_CONTROL_TOGGLE,
        "Faster Game Over Screen",
        "Makes the Game Over screen not wait for the music to finish before "
        "showing the prompt to continue."
        "<br/><br/>"
        "Pairs well with the Turbo button in the main Dusklight settings."
    );
    addControl(left, control);

    svc_ui->pane_add_section(mod_ctx, left, "Visuals");

    svc_ui->pane_add_section(mod_ctx, left, "Audio");

    return MOD_OK;
}

static void onControlsWindowClosed(ModContext*, UiWindowHandle, void*) {
    controlsWindowHandle = 0;
}

static void onOpenControls(ModContext*, void*) {
    if (controlsWindowHandle != 0) {
        return;
    }
    UiTabDesc tabs[1] = { UI_TAB_DESC_INIT };
    tabs[0].title = "Settings";
    tabs[0].build = buildSettingsTab;
    UiWindowDesc desc = UI_WINDOW_DESC_INIT;
    desc.tabs = tabs;
    desc.tab_count = 1;
    desc.on_closed = onControlsWindowClosed;
    if (svc_ui->window_push(mod_ctx, &desc, &controlsWindowHandle) != MOD_OK) {
        mods::log::error("failed to open settings window");
    }
}

static ModResult buildModPanel(ModContext*, UiElementHandle panel, void*, ModError*) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_BUTTON;
    control.label = "Open Settings";
    control.on_pressed = onOpenControls;
    addControl(panel, control);
    return MOD_OK;
}

static ModResult registerBoolOption(
    const char* name, bool defaultValue, ConfigVarHandle& outHandle) {
    ConfigVarDesc cvarDesc = CONFIG_VAR_DESC_INIT;
    cvarDesc.name = name;
    cvarDesc.type = CONFIG_VAR_BOOL;
    cvarDesc.default_bool = defaultValue;
    ModResult result = svc_config->register_var(mod_ctx, &cvarDesc, &outHandle);
    if (result != MOD_OK) {
        mods::log::error("failed to register option");
        return result;
    }
    return MOD_OK;
}

static ModResult registerIntOption(
    const char* name, int64_t defaultValue, ConfigVarHandle& outHandle) {
    ConfigVarDesc cvarDesc = CONFIG_VAR_DESC_INIT;
    cvarDesc.name = name;
    cvarDesc.type = CONFIG_VAR_INT;
    cvarDesc.default_int = defaultValue;
    ModResult result = svc_config->register_var(mod_ctx, &cvarDesc, &outHandle);
    if (result != MOD_OK) {
        mods::log::error("failed to register option");
        return result;
    }
    return MOD_OK;
}

static int64_t getInt(ConfigVarHandle handle) {
    int64_t result;
    svc_config->get_int(mod_ctx, handle, &result);
    return result;
}

static bool getBool(ConfigVarHandle handle) {
    bool result;
    svc_config->get_bool(mod_ctx, handle, &result);
    return result;
}

ModResult Config::init() {
    registerBoolOption("wolfLinkSwimming", true, wolfLinkSwimmingHandle);
    registerIntOption("maxOxygen", 40, maxOxygenHandle);
    registerBoolOption("useYToSwim", false, useYHandle);
    registerBoolOption("keepCameraOnDeath", false, keepCameraOnDeathHandle);
    registerBoolOption("fasterGameOver", false, fasterGameOverHandle);

    UiModsPanelDesc panelDesc = UI_MODS_PANEL_DESC_INIT;
    panelDesc.build = buildModPanel;
    svc_ui->register_mods_panel(mod_ctx, &panelDesc);

    return MOD_OK;
}

bool Config::getWolfLinkSwimming() { return getBool(wolfLinkSwimmingHandle); }
s32 Config::getMaxOxygen() { return (s32)getInt(maxOxygenHandle); }
bool Config::getUseYToSwim() { return getBool(useYHandle); }
bool Config::getKeepCameraOnDeath() { return getBool(keepCameraOnDeathHandle); }
bool Config::getFasterGameOver() { return getBool(fasterGameOverHandle); }
