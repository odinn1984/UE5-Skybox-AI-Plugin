#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BlockadeLabsSkyboxAiSettings.generated.h"

UCLASS(config = "BlackdaleLabs", meta = (DisplayName = "Blockade Labs Skybox AI"))
class UBlockadeLabsSkyboxAiSettings : public UDeveloperSettings
{
  GENERATED_BODY()

public:
  UPROPERTY(config, EditAnywhere, Category = "Blockdale Labs Skybox API")
  FString ApiKey;

  UPROPERTY(config, EditAnywhere, Category = "Blockdale Labs Skybox API")
  FString ApiEndpoint = TEXT("https://backend.blockadelabs.com/api/v1");

  UPROPERTY(config, EditAnywhere, Category = "Blockdale Labs Skybox API")
  FString SaveDirectory = TEXT("/Game/Content/SkyboxAI/Exports");

  UBlockadeLabsSkyboxAiSettings(const FObjectInitializer &ObjectInitializer);
};
