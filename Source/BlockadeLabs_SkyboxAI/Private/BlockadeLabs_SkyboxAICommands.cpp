// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlockadeLabs_SkyboxAICommands.h"

#define LOCTEXT_NAMESPACE "FBlockadeLabs_SkyboxAIModule"

void FBlockadeLabs_SkyboxAICommands::RegisterCommands()
{
  UI_COMMAND(
    OpenPluginWindow,
    "BlockadeLabs_SkyboxAI",
    "Bring up BlockadeLabs SkyboxAI window",
    EUserInterfaceActionType::Button,
    FInputChord()
    );
}

#undef LOCTEXT_NAMESPACE
