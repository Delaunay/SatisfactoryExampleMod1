#include "ExampleModModule.h"
#include "mod/blueprint_hooking.h"
#include "util/Logging.h"

#include "Player/CharacterPlayerExtension.h"

void FExampleModModule::StartupModule() {
    SML::Logging::debug(TEXT("Registering RPG Module"));

    UCharacterPlayerExtension::Register();
}

IMPLEMENT_GAME_MODULE(FExampleModModule, ExampleMod);
