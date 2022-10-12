// ParticleEditor.cpp : Определяет точку входа для приложения.

#include "stdafx.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    if (!IsDebuggerPresent())
        Debug._initialize(false);
    Core._initialize("particle", ELogCallback, 1, "fs.ltx", true);

    PTools = xr_new<CParticleTool>();
    Tools = PTools;

    UI = xr_new<CParticleMain>();
    UI->RegisterCommands();

    UIMainForm *MainForm = xr_new<UIMainForm>();
    ::MainForm = MainForm;
    UI->Push(MainForm, false);

    while (MainForm->Frame())
    {
    }
    
    xr_delete(MainForm);
    Core._destroy();
    return 0;
}
