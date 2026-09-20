#include "config.h"
#include "mods/api.h"
#include "mods/svc/config.h"
#include "mods/svc/log.hpp"
#include "mods/svc/ui.h"

static UiWindowHandle controlsWindowHandle;
static ConfigVarHandle wolfLinkSwimmingHandle;

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

void addControl(UiElementHandle pane, const UiControlDesc& desc) {
    svc_ui->pane_add_control(mod_ctx, pane, &desc, nullptr);
}

ModResult buildSettingsTab(
    ModContext*, UiWindowHandle, UiElementHandle left, UiElementHandle right, 
    void*, ModError*) {

    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_TOGGLE;
    control.label = "Enable Wolf Link Diving";
    control.help_rml = "Allows you to dive underwater as Wolf Link.<br/><br/>"
                       "By default, press B to swim down, and A to swim up.";
    control.binding = UI_BINDING_CONFIG_VAR;
    control.config_var = wolfLinkSwimmingHandle;
    addControl(left, control);

    return MOD_OK;
}

void on_controls_window_closed(ModContext*, UiWindowHandle, void*) {
    controlsWindowHandle = 0;
}

void onOpenControls(ModContext*, void*) {
    if (controlsWindowHandle != 0) {
        return;
    }
    UiTabDesc tabs[1] = { UI_TAB_DESC_INIT };
    tabs[0].title = "Settings";
    tabs[0].build = buildSettingsTab;
    UiWindowDesc desc = UI_WINDOW_DESC_INIT;
    desc.tabs = tabs;
    desc.tab_count = 1;
    desc.on_closed = on_controls_window_closed;
    if (svc_ui->window_push(mod_ctx, &desc, &controlsWindowHandle) != MOD_OK) {
        mods::log::error("failed to open settings window");
    }
}

ModResult buildModPanel(ModContext*, UiElementHandle panel, void*, ModError*) {
    UiControlDesc control = UI_CONTROL_DESC_INIT;
    control.kind = UI_CONTROL_BUTTON;
    control.label = "Open Settings";
    control.on_pressed = onOpenControls;
    addControl(panel, control);
    return MOD_OK;
}

ModResult Config::init()
{
    registerBoolOption("wolfLinkSwimming", true, wolfLinkSwimmingHandle);

    UiModsPanelDesc panelDesc = UI_MODS_PANEL_DESC_INIT;
    panelDesc.build = buildModPanel;
    svc_ui->register_mods_panel(mod_ctx, &panelDesc);

    return MOD_OK;
}

bool Config::getWolfLinkSwimming()
{
    bool result;
    svc_config->get_bool(mod_ctx, wolfLinkSwimmingHandle, &result);
    return result;
}
