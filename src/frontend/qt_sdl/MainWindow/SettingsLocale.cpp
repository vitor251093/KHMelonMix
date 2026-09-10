/*
    Copyright 2016-2025 melonDS team

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#include "SettingsLocale.h"

static const SettingsLocale kEnglish = {
    /* on */  "On",
    /* off */ "Off",
    /* none */ "None",
    /* titleGameSettings */ "GAME SETTINGS",

    /* sidebarLabels */ {
        "Game",
        "Emulation",
        "Display",
        "Sound",
        "Gamepad",
        "Keyboard",
        "System",
        "Before You Stream\xE2\x80\xA6",
        "Quit Game",
    },

    /* sectionOverview */ {
        "Settings specific to the loaded Kingdom Hearts game.",
        "Configure console type, frame rate, and runtime behavior.",
        "Renderer, resolution, synchronization, and visual theme.",
        "Volume, audio quality, and microphone input.",
        "Remap controller buttons and analog stick assignments.",
        "Remap keyboard bindings for DS buttons and hotkeys.",
        "Network mode, battery simulation, and firmware options.",
        "Streaming guidelines and copyright information.",
        "Exit the current game session.",
    },

    /* hintToggle */      "Toggle",
    /* hintSelect */      "Select",
    /* hintAdjust */      "Adjust",
    /* hintReset */       "Reset",
    /* hintBack */        "Back",
    /* hintEnter */       "Enter",
    /* hintClose */       "Close",
    /* hintConfirm */     "Confirm",
    /* hintRebind */      "Rebind",
    /* hintResetAll */    "Reset All",
    /* hintClear */       "Clear",
    /* remapSelectPrompt */  "Select an action to assign.",
    /* remapResetBackHint */ "X Reset All  |  B Back to Sidebar",
    /* hintOpenWebsite */ "Open Website",
    /* hintQuitGame */    "Quit Game",
    /* hintYes */         "Yes",
    /* hintNo */          "No",
    /* hintCancel */      "Cancel",

    /* captureController */     "Press a controller button\xE2\x80\xA6",
    /* captureKey */            "Press a key\xE2\x80\xA6",
    /* captureCancelKeyboard */ "Press Esc to cancel",
    /* captureReleaseHint */    "Release controller\xE2\x80\xA6",
    /* captureCancelCountdown */"Cancelling in %1s",

    /* resetSectionPrompt */ "Reset %1 to defaults?",
    /* resetAllBindings */   "Reset all bindings?",
    /* pillYes */ "Yes",
    /* pillNo */  "No",

    /* noKHGameNote */
        "No Kingdom Hearts game is running \xe2\x80\x94 "
        "KH-specific settings will appear here once a game is loaded.",

    /* gameLanguageLabel */       "Language",
    /* gameLanguageDesc */        "Firmware language used for cutscene subtitles and pause menus.",
    /* gameFastForwardLabel */    "Fast-forward Loading",
    /* gameFastForwardDesc */     "Speed through loading screens automatically.",
    /* gameSkipCutscenesLabel */  "Skip Cutscenes Instantly",
    /* gameSkipCutscenesDesc */   "Press a button during a cutscene to skip it immediately.",
    /* gameHisMemoriesLabel */    "Disable \"His\" Memories",
    /* gameHisMemoriesDesc */     "Remove the additional memory episodes added by this port.",

    /* displayEnhancedLabel */     "Enhanced Graphics",
    /* displayEnhancedDesc */      "Enable high-resolution backgrounds and models.",
    /* displaySingleScreenLabel */ "Single Screen Mode",
    /* displaySingleScreenDesc */  "Display only the top screen, scaled to fill the window.",
    /* displayHDCutscenesLabel */  "Enable HD Cutscenes",
    /* displayHDCutscenesDesc */   "Play pre-rendered HD cutscenes instead of the original ingame cutscenes, when available.",
    /* displaySubtitlesLabel */    "Show Subtitles",
    /* displaySubtitlesDesc */     "Display subtitles during HD cutscene playback.",
    /* displayHUDScaleLabel */     "HUD Scale",
    /* displayHUDScaleDesc */      "Scale the heads-up display elements.",

    /* soundAudioPackLabel */ "Audio Pack",
    /* soundAudioPackDesc */  "Replacement music pack for in-game audio.",

    /* gamepadConfirmBtnLabel */ "Confirm",
    /* gamepadConfirmBtnDesc */  "Assign which DS button should be the confirmation button.",
    /* gamepadBindingsLabel */ "Key Bindings",
    /* gamepadBindingsDesc */  "Assign gamepad keys to DS buttons and hotkeys.",
    /* gamepadCamSensLabel */  "Camera Sensitivity",
    /* gamepadCamSensDesc */   "Speed of the touch-screen camera controls.",

    /* keyboardBindingsLabel */ "Key Bindings",
    /* keyboardBindingsDesc */  "Assign keyboard keys to DS buttons and hotkeys.",

    // Emulation
    /* emuConsoleType */     "Console Type",          "Emulated console. Changes take effect after restarting the game.",
    /* emuDirectBoot */      "Direct Boot",           "Skip the DS boot sequence and launch the game directly.",
    /* emuFpsLimit */        "FPS Limit",             "Cap emulation speed to the target frame rate.",
    /* emuTargetFps */       "Target FPS",            "Frame rate cap for normal play.",
    /* emuFastForwardFps */  "Fast-forward FPS",      "Speed cap while fast-forward is held.",
    /* emuSlowmoFps */       "Slowmo FPS",            "Frame rate cap while slow-motion is active.",
    /* emuMuteFastForward */ "Mute Fast-forward",     "Silence audio while fast-forward is held.",
    /* emuPauseLostFocus */  "Pause on Lost Focus",   "Pause emulation when the window loses focus.",
    /* emuHideMouse */       "Hide Mouse",            "Hide the mouse cursor while the emulator window is active.",
    /* emuHideMouseAfter */  "Hide Mouse After",      "Automatically hide the cursor after this period of inactivity.",
    /* emuJit */             "JIT Recompiler",        "Improves performance but may reduce stability. Takes effect on the next ROM load.",
    // Display
    /* displayRenderer */    "3D Renderer",           "Graphics renderer for 3D scenes. OpenGL modes require GPU support.",
    /* displayResolution */  "3D Resolution",         "Internal rendering scale for 3D graphics.",
    /* displayVSync */       "VSync",                 "Synchronize rendering to your display refresh rate.",
    /* displayThemeColor */  "Theme Color",           "Settings menu accent color inspired by each Kingdom Hearts title.",
    // Sound
    /* soundVolume */        "Volume",                "Game audio output level.",
    /* soundBgmVolume */     "BGM Volume",            "Background music audio output level.",
    /* soundInterpolation */ "Interpolation",         "Audio resampling quality. Higher settings improve fidelity but use more CPU.",
    /* soundBitDepth */      "Bit Depth",             "Audio sample bit depth. 10-bit mimics original DS hardware.",
    /* soundDSiVolumeSync */ "DSi Volume Sync",       "Sync DS audio volume with the DSi firmware volume level.",
    /* soundMicInput */      "Mic Input",             "Microphone input source for games that use the DS microphone.",
    // System
    /* systemWifiMode */     "WiFi Mode",             "Connection mode for wireless features.",
    /* systemWifiAdapter */  "WiFi Adapter",          "Network adapter used for Direct Mode. Only active when Direct is selected.",
    /* systemDSBattery */    "DS Battery",            "Simulated DS battery level for games that check it.",
    /* systemDSiBattery */   "DSi Battery Level",     "Simulated DSi battery charge level.",
    /* systemDSiCharging */  "DSi Charging",          "Whether the simulated DSi battery is charging.",

    /* streamPara1 */
        "In order to stream Melon Mix with OBS, capture your screen or it won't record "
        "the cutscenes, and capture your whole audio or it won't record the replacement BGM.",
    /* streamPara2 */
        "This game is a copyrighted work. The copyright is held by The Walt Disney "
        "Company and a collaboration of authors representing The Walt Disney Company. "
        "Additionally, the copyright of certain characters is held by Square Enix Co., Ltd.",
    /* streamPara3 */
        "You are free to stream this game in non-commercial contexts. However, using "
        "streams of the game to primarily provide or listen to the music is prohibited "
        "even in such non-commercial contexts.",
    /* streamPara4 */
        "For information on the terms of use relating to streaming the game, please see "
        "the official KINGDOM HEARTS site.",
    /* streamOpenHint */ "Open a browser to view the official website.",
    /* quitBody */
        "You are about to quit the current game. "
        "Any progress since your last save will be lost.",
};

static const SettingsLocale kSpanish = {
    /* on */  "Sí",
    /* off */ "No",
    /* none */ "Nada",
    /* titleGameSettings */ "AJUSTES DE LOS JUEGOS",

    /* sidebarLabels */ {
        "Juego",
        "Emulación",
        "Gráficos",
        "Sonido",
        "Mando",
        "Teclado",
        "Sistema",
        "Antes de retransmitir\xE2\x80\xA6",
        "Salir",
    },

    /* sectionOverview */ {
        "Establece ajustes específicos del juego de Kingdom Hearts cargado.",
        "Establece el tipo de consola, la frecuencia de fotogramas y el comportamiento del entorno de ejecución.",
        "Establece el renderizador, la resolución, la sincronización y el tema visual.",
        "Establece el volumen, la calidad del audio y la entrada del micrófono.",
        "Reasigna los botones y las palancas analógicas del mando.",
        "Reasigna las teclas del teclado correspondientes a botones de la DS y atajos.",
        "Establece el modo de red, la simulación de la batería y las opciones del firmware.",
        "Consulta los lineamientos de retransmisión y la información sobre derechos de autor.",
        "Sal de la sesión del juego actual.",
    },

    /* hintToggle */      "Cambiar",
    /* hintSelect */      "Seleccionar",
    /* hintAdjust */      "Ajustar",
    /* hintReset */       "Restablecer",
    /* hintBack */        "Volver",
    /* hintEnter */       "Ingresar",
    /* hintClose */       "Cerrar",
    /* hintConfirm */     "Confirmar",
    /* hintRebind */      "Reasignar",
    /* hintResetAll */    "Restablecer todo",
    /* hintClear */       "Borrar",
    /* remapSelectPrompt */  "Selecciona la acción que quieras asignar.",
    /* remapResetBackHint */ "X Restablecer todo  |  B Volver a la barra lateral",
    /* hintOpenWebsite */ "Abrir el sitio web",
    /* hintQuitGame */    "Salir del juego",
    /* hintYes */         "Sí",
    /* hintNo */          "No",
    /* hintCancel */      "Cancelar",

    /* captureController */     "Presiona un botón del mando\xE2\x80\xA6",
    /* captureKey */            "Presiona una tecla\xE2\x80\xA6",
    /* captureCancelKeyboard */ "Presiona Esc para cancelar",
    /* captureReleaseHint */    "Libera el mando\xE2\x80\xA6",
    /* captureCancelCountdown */"Cancelando en %1s",

    /* resetSectionPrompt */ "¿Quieres restablecer %1 al valor predeterminado?",
    /* resetAllBindings */   "¿Quieres restablecer todas las asignaciones?",
    /* pillYes */ "Sí",
    /* pillNo */  "No",

    /* noKHGameNote */
        "No se está ejecutando ningún juego de Kingdom Hearts. "
        "Aquí aparecerán ajustes específicos de KH cuando hayas cargado un juego.",

    /* gameLanguageLabel */       "Idioma",
    /* gameLanguageDesc */        "Establece el idioma del firmware, que se usa para los subtítulos de las escenas y los menús de pausa.",
    /* gameFastForwardLabel */    "Acelerar carga",
    /* gameFastForwardDesc */     "Acelera automáticamente las pantallas de carga.",
    /* gameSkipCutscenesLabel */  "Saltar escenas instantáneamente",
    /* gameSkipCutscenesDesc */   "Cuando presiones un botón durante una escena, la saltarás de inmediato.",
    /* gameHisMemoriesLabel */    "Inhabilitar recuerdos",
    /* gameHisMemoriesDesc */     "Quita los episodios de recuerdos adicionales que se agregaron en esta versión.",

    /* displayEnhancedLabel */     "Gráficos mejorados",
    /* displayEnhancedDesc */      "Habilita fondos y modelos de alta resolución.",
    /* displaySingleScreenLabel */ "Modo de una sola pantalla",
    /* displaySingleScreenDesc */  "Muestra solo la pantalla superior de la DS y la redimensiona al tamaño de tu pantalla.",
    /* displayHDCutscenesLabel */  "Habilitar escenas en HD",
    /* displayHDCutscenesDesc */   "Reproduce escenas prerrenderizadas en HD en vez de las escenas originales del juego (si están disponibles).",
    /* displaySubtitlesLabel */    "Mostrar subtítulos",
    /* displaySubtitlesDesc */     "Muestra subtítulos durante la reproducción de las escenas en HD.",
    /* displayHUDScaleLabel */     "Escala de la interfaz",
    /* displayHUDScaleDesc */      "Ajusta el tamaño de los elementos de la interfaz.",

    /* soundAudioPackLabel */ "Paquete de audio",
    /* soundAudioPackDesc */  "Establece el paquete de reemplazo de música para el audio del juego.",

    /* gamepadConfirmBtnLabel */ "Confirmar",
    /* gamepadConfirmBtnDesc */  "Asigna el botón de la DS que se usará para confirmar acciones.",
    /* gamepadBindingsLabel */ "Asignación de botones",
    /* gamepadBindingsDesc */  "Asigna botones del mando a botones de la DS y atajos.",
    /* gamepadCamSensLabel */  "Sensibilidad de la cámara",
    /* gamepadCamSensDesc */   "Establece la velocidad de los controles táctiles de la cámara.",

    /* keyboardBindingsLabel */ "Asignación de teclas",
    /* keyboardBindingsDesc */  "Asigna teclas del teclado a botones de la DS y atajos.",

    // Emulation
    /* emuConsoleType */     "Tipo de consola",          "Establece la consola emulada. Surte efecto después de reiniciar el juego.",
    /* emuDirectBoot */      "Inicio directo",           "Salta la secuencia de encendido de la DS para iniciar el juego directamente.",
    /* emuFpsLimit */        "Límite de FPS",             "Limita la velocidad de emulación a la frecuencia de fotogramas objetivo.",
    /* emuTargetFps */       "FPS objetivo",            "Limita la frecuencia de fotogramas de la jugabilidad habitual.",
    /* emuFastForwardFps */  "FPS para adelantar",      "Limita la velocidad mientras se mantiene presionado el botón para adelantar.",
    /* emuSlowmoFps */       "FPS para ralentizar",            "Limita la frecuencia de fotogramas mientras está activa la ralentización.",
    /* emuMuteFastForward */ "Silenciar al adelantar",     "Silencia el audio mientras se mantiene presionado el botón para adelantar.",
    /* emuPauseLostFocus */  "Pausar en segundo plano",   "Pausa la emulación cuando la ventana no está activa.",
    /* emuHideMouse */       "Ocultar cursor",            "Oculta el cursor del mouse mientras la ventana del emulador está activa.",
    /* emuHideMouseAfter */  "Tiempo para ocultar el cursor",      "Oculta automáticamente el cursor del mouse después de este período de inactividad.",
    /* emuJit */             "Recompilador JIT",        "Mejora el rendimiento a costa de la estabilidad. Surte efecto la próxima vez que cargues la ROM.",
    // Display
    /* displayRenderer */    "Renderizador 3D",           "Establece el renderizador gráfico de las pantallas 3D. Para usar los modos de OpenGL, se requiere una GPU compatible.",
    /* displayResolution */  "Resolución 3D",         "Establece la escala de renderización interna de los gráficos 3D.",
    /* displayVSync */       "Sincronización vertical",                 "Sincroniza el renderizado a la frecuencia de actualización de tu pantalla.",
    /* displayThemeColor */  "Tema de color",           "Establece los colores del menú según el juego de Kingdom Hearts.",
    // Sound
    /* soundVolume */        "Volumen",                "Establece el nivel de salida del audio del juego.",
    /* soundBgmVolume */     "Volumen de la música",            "Establece el nivel de salida de la música de fondo.",
    /* soundInterpolation */ "Interpolación",         "Establece la calidad de remuestreo del audio. Los ajustes de mayor calidad mejoran la fidelidad, pero utilizan más la CPU.",
    /* soundBitDepth */      "Profundidad de bits",             "Establece la profundidad de bits de las muestras de audio. La opción de 10 bits imita el hardware original de la DS.",
    /* soundDSiVolumeSync */ "Sincronización de volumen de la DSi",       "Sincroniza el volumen del audio de la DS con nivel de volumen del firmware de la DSi.",
    /* soundMicInput */      "Entrada del micrófono",             "Establece la fuente de entrada para los juegos que utilizan el micrófono de la DS.",
    // System
    /* systemWifiMode */     "Modo de Wi-Fi",             "Establece el modo de conexión de las funciones inalámbricas.",
    /* systemWifiAdapter */  "Adaptador Wi-Fi",          "Establece el adaptador de red que se utiliza para el modo directo (solo activo cuando ese modo está seleccionado).",
    /* systemDSBattery */    "Batería de la DS",            "Establece el nivel de la batería de la DS para los juegos que lo comprueban.",
    /* systemDSiBattery */   "Nivel de la batería de la DSi",     "Establece el nivel de carga de la batería simulada de la DSi.",
    /* systemDSiCharging */  "Carga de la DSi",          "Establece si la batería simulada de la DSi se está cargando.",

    /* streamPara1 */
        "Para retransmitir Melon Mix con OBS, captura toda la pantalla (de lo contrario, "
        "no se grabarán las escenas). Además, captura todo el audio del sistema, o no se grabará la música de reemplazo.",
    /* streamPara2 */
        "Este juego es una obra protegida por derechos de autor propiedad de The Walt Disney "
        "Company y una colaboración de autores que representan a The Walt Disney Company. "
        "Además, los derechos de autor de ciertos personajes pertenecen a Square Enix Co., Ltd.",
    /* streamPara3 */
        "La retransmisión de este juego queda permitida únicamente con fines no comerciales. "
        "No obstante, la reproducción del juego para proporcionar o escuchar la música "
        "como fin principal queda prohibida incluso para uso no comercial.",
    /* streamPara4 */
        "Para obtener más información acerca de las condiciones de uso en relación "
        "con la reproducción del juego, visita el sitio oficial de Kingdom Hearts. ",
    /* streamOpenHint */ "Abre un navegador para ver el sitio web oficial.",
    /* quitBody */
        "Saldrás del juego actual. "
        "Perderás el progreso logrado después de la última vez que guardaste.",
};

// TODO: KH Add support to all languages from PluginLanguage.h
const SettingsLocale kLocales[6] = {
    kEnglish,  // 0  English
    kEnglish,  // 1  Japanese  (stub — fill in fields to localize)
    kEnglish,  // 2  French    (stub)
    kEnglish,  // 3  German    (stub)
    kEnglish,  // 4  Italian   (stub)
    kSpanish,  // 5  Spanish   (WIP)
};
