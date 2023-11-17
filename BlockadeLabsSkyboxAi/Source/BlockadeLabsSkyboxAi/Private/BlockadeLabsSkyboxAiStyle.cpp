// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlockadeLabsSkyboxAiStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FBlockadeLabsSkyboxAiStyle::StyleInstance = nullptr;

void FBlockadeLabsSkyboxAiStyle::Initialize()
{
  if (!StyleInstance.IsValid())
  {
    StyleInstance = Create();
    FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
  }
}

void FBlockadeLabsSkyboxAiStyle::Shutdown()
{
  FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
  ensure(StyleInstance.IsUnique());
  StyleInstance.Reset();
}

FName FBlockadeLabsSkyboxAiStyle::GetStyleSetName()
{
  static FName StyleSetName(TEXT("BlockadeLabsSkyboxAiStyle"));
  return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef<FSlateStyleSet> FBlockadeLabsSkyboxAiStyle::Create()
{
  TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("BlockadeLabsSkyboxAiStyle"));
  Style->SetContentRoot(IPluginManager::Get().FindPlugin("BlockadeLabsSkyboxAi")->GetBaseDir() / TEXT("Resources"));

  Style->Set("BlockadeLabsSkyboxAi.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

  return Style;
}

void FBlockadeLabsSkyboxAiStyle::ReloadTextures()
{
  if (FSlateApplication::IsInitialized())
  {
    FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
  }
}

const ISlateStyle& FBlockadeLabsSkyboxAiStyle::Get()
{
  return *StyleInstance;
}
