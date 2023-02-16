// Remaps CapsLock to Control
// Future plans:
//      - remap Control to Windows only on IBM
//      - add tapped escape function

#include "interception.h"
#include "utils.h"

#include <iostream>
#include <cstring>
// #include <windows.h>

InterceptionKeyStroke rcontrol_down  = { 0x1D, INTERCEPTION_KEY_DOWN | INTERCEPTION_KEY_E0 };
InterceptionKeyStroke lcontrol_down  = { 0x1D, INTERCEPTION_KEY_DOWN };
InterceptionKeyStroke windows_down  = { 0x5B, INTERCEPTION_KEY_DOWN | INTERCEPTION_KEY_E0 };
InterceptionKeyStroke capslock_down = { 0x3A, INTERCEPTION_KEY_DOWN };
InterceptionKeyStroke rcontrol_up    = { 0x1D, INTERCEPTION_KEY_UP | INTERCEPTION_KEY_E0 };
InterceptionKeyStroke lcontrol_up    = { 0x1D, INTERCEPTION_KEY_UP };
InterceptionKeyStroke windows_up    = { 0x5B, INTERCEPTION_KEY_UP | INTERCEPTION_KEY_E0 };
InterceptionKeyStroke capslock_up   = { 0x3A, INTERCEPTION_KEY_UP };

const wchar_t* HARDWARE_ID_LENOVO = L"ACPI\\VEN_LEN&DEV_0071";
const wchar_t* HARDWARE_ID_MODELM = L"HID\\VID_FEED&PID_B155&REV_010&MI_00";

bool operator == (const InterceptionKeyStroke &first, const InterceptionKeyStroke &second)
{
    return first.code == second.code && first.state == second.state;
}
bool operator != (const InterceptionKeyStroke &first, const InterceptionKeyStroke &second)
{
    return !(first == second);
}

InterceptionKeyStroke control_to_windows(InterceptionKeyStroke keystroke) {
    if(keystroke == rcontrol_down) keystroke.code = windows_down.code;
    if(keystroke == lcontrol_down) keystroke = windows_down;
    if(keystroke == rcontrol_up) keystroke.code = windows_up.code;
    if(keystroke == lcontrol_up) keystroke = windows_up;
    return keystroke;
}

InterceptionKeyStroke caps_to_control(InterceptionKeyStroke keystroke) {
    if(keystroke == capslock_down) keystroke.code = lcontrol_down.code;
    if(keystroke == capslock_up) keystroke.code = lcontrol_up.code;
    return keystroke;
}

int main()
{
    using namespace std;

    InterceptionContext context;
    InterceptionDevice device;
    InterceptionStroke stroke;

    wchar_t hardware_id[500];

    raise_process_priority();

    context = interception_create_context();

    interception_set_filter(context, interception_is_keyboard, INTERCEPTION_FILTER_KEY_ALL);

    while(interception_receive(context, device = interception_wait(context), &stroke, 1) > 0)
    {
        interception_get_hardware_id(context, device, hardware_id, sizeof(hardware_id));

        int is_lenovo = wcscmp(HARDWARE_ID_LENOVO, hardware_id);

        if(interception_is_keyboard(device))
        {
            InterceptionKeyStroke &keystroke = *(InterceptionKeyStroke *) &stroke;
            if(is_lenovo == -1) {
                if(keystroke.code == 0x1D) keystroke = control_to_windows(keystroke);
            }
            if(keystroke.code == 0x3A) keystroke = caps_to_control(keystroke);
        }

        interception_send(context, device, &stroke, 1);
    }

    interception_destroy_context(context);

    return 0;
}
