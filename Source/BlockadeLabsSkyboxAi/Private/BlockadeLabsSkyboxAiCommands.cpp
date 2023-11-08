// Copyright Epic Games, Inc. All Rights Reserved.

#include "..\Public\BlockadeLabsSkyboxAiCommands.h"

#define LOCTEXT_NAMESPACE "FBlockadeLabsSkyboxAiModule"

void FBlockadeLabsSkyboxAiCommands::RegisterCommands()
{
  UI_COMMAND(
    OpenPluginWindow,
    "BlockadeLabsSkyboxAi",
    "Bring up BlockadeLabs SkyboxAI window",
    EUserInterfaceActionType::Button,
    FInputChord()
    );
}

#undef LOCTEXT_NAMESPACE
