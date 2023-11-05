#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "BlockadeLabs_SkyboxAISettings.generated.h"

UCLASS(config = "BlackdaleLabs", meta = (DisplayName = "Blockade Labs Skybox AI"))
class UBlockadeLabs_SkyboxAISettings : public UDeveloperSettings
{
  GENERATED_BODY()

public:
  UPROPERTY(config, EditAnywhere, Category = "Blockdale Labs Skybox API")
  FString APIKey;

  UPROPERTY(config, EditAnywhere, Category = "Blockdale Labs Skybox API")
  FString APIEndpoint = TEXT("https://backend.blockadelabs.com/api/v1");

  UPROPERTY(config, EditAnywhere, Category = "Blockdale Labs Skybox API")
  FString SaveDirectory = TEXT("/Game/Content/SkyboxAI/Exports");

  UBlockadeLabs_SkyboxAISettings(const FObjectInitializer &ObjectInitializer);

  virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;
};
