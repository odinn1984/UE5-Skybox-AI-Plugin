#include "..\Public\BlockadeLabs_SkyboxAISettings.h"

UBlockadeLabs_SkyboxAISettings::UBlockadeLabs_SkyboxAISettings(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
}

void UBlockadeLabs_SkyboxAISettings::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
  Super::PostEditChangeProperty(PropertyChangedEvent);

  if (PropertyChangedEvent.Property->GetName() == "SaveDirectory")
  {
  }
}
