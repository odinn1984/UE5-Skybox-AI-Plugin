// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlockadeLabs_SkyboxAIStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FBlockadeLabs_SkyboxAIStyle::StyleInstance = nullptr;

void FBlockadeLabs_SkyboxAIStyle::Initialize()
{
  if (!StyleInstance.IsValid())
  {
    StyleInstance = Create();
    FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
  }
}

void FBlockadeLabs_SkyboxAIStyle::Shutdown()
{
  FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
  ensure(StyleInstance.IsUnique());
  StyleInstance.Reset();
}

FName FBlockadeLabs_SkyboxAIStyle::GetStyleSetName()
{
  static FName StyleSetName(TEXT("BlockadeLabs_SkyboxAIStyle"));
  return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef<FSlateStyleSet> FBlockadeLabs_SkyboxAIStyle::Create()
{
  TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("BlockadeLabs_SkyboxAIStyle"));
  Style->SetContentRoot(IPluginManager::Get().FindPlugin("BlockadeLabs_SkyboxAI")->GetBaseDir() / TEXT("Resources"));

  Style->Set("BlockadeLabs_SkyboxAI.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

  return Style;
}

void FBlockadeLabs_SkyboxAIStyle::ReloadTextures()
{
  if (FSlateApplication::IsInitialized())
  {
    FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
  }
}

const ISlateStyle &FBlockadeLabs_SkyboxAIStyle::Get()
{
  return *StyleInstance;
}
