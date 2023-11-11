﻿#include "SSkyboxAiWidget.h"
#include "BlockadeLabsSkyboxAiSettings.h"
#include "ImagineProvider.h"
#include "Framework/Notifications/NotificationManager.h"
#include "SkyboxAiApi.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"

DEFINE_LOG_CATEGORY(SkyboxAiWidget);

void SSkyboxAiWidget::Construct(const FArguments &InArgs)
{
  if (!SkyboxApi.IsValid()) SkyboxApi = NewObject<USkyboxApi>();

  if (!PluginSettings.IsValid())
  {
    PluginSettings = NewObject<UBlockadeLabsSkyboxAiSettings>();
    PluginSettings->OnSettingChanged().AddLambda(
      [this](UObject *Object, struct FPropertyChangedEvent &Event)
      {
        if (Event.GetPropertyName() != GET_MEMBER_NAME_CHECKED(
          UBlockadeLabsSkyboxAiSettings,
          bEnablePremiumContent
          ))
          return;
        if (!EnhancePromptCheckbox.IsValid() || !PluginSettings.IsValid()) return;

        if (!PluginSettings->bEnablePremiumContent) EnhancePromptCheckbox->SetIsChecked(ECheckBoxState::Unchecked);
        EnhancePromptCheckbox->SetEnabled(PluginSettings->bEnablePremiumContent);

        OnRefreshLists();
      }
      );
  }

  WidgetData = FSkyboxAiWidgetData(
    InArgs._bEnrichPrompt,
    InArgs._Prompt,
    InArgs._NegativeText,
    InArgs._Category,
    InArgs._ExportType
    );

  OnSkyboxAiWidgetDataChanged = InArgs._OnSkyboxAiWidgetDataChanged;

  const TSharedPtr<SVerticalBox> Widget = SNew(SVerticalBox);

  AddPrompt(Widget);
  AddNegativeTextAndCategories(Widget);
  AddBottomRow(Widget);

  ChildSlot
    [Widget.ToSharedRef()];

  OnRefreshLists();
}

void SSkyboxAiWidget::LoadCategoriesFromList(const FSkyboxAiStyles &List)
{
  LoadViewListFromMap<FSkyboxListEntry, FSkyboxAiStylesTuple>(
    Categories,
    FilteredCategories,
    CategoryListView,
    List,
    WidgetData.Category,
    TEXT("Categories"),
    [this](const FSkyboxListEntry &Item)
    {
      return FText::FromString(Item.Name);
    }
    );
}

void SSkyboxAiWidget::LoadExportTypesFromList(const FSkyboxAiExportTypes &List)
{
  LoadViewListFromMap<FString, FSkyboxAiExportTypesTuple>(
    ExportTypes,
    FilteredExportTypes,
    ExportTypeListView,
    List,
    WidgetData.ExportType,
    TEXT("Export Types"),
    [this](const FString &Item)
    {
      return FText::FromString(Item);
    }
    );
}

void SSkyboxAiWidget::AddPrompt(TSharedPtr<SVerticalBox> RootWidget)
{
  TSharedPtr<SHorizontalBox> HBox = SNew(SHorizontalBox);

  HBox->AddSlot()
      .HAlign(HAlign_Left)
      .VAlign(VAlign_Center)
      .Padding(10.0f)
      .AutoWidth()
  [
    SAssignNew(
      PromptLabel,
      STextBlock
      ).MinDesiredWidth(125)
       .Text(
         FText::FromString(
           FString::Printf(
             TEXT("Prompt\r\n[0 / %d]"),
             std::get<TUPLE_PROMPT_MAX_LEN_IDX>(WidgetData.Category)
             )
           )
         )
  ];

  HBox->AddSlot()
      .HAlign(HAlign_Fill)
      .VAlign(VAlign_Fill)
      .Padding(10.0f)
      .FillWidth(1.0)
  [
    SAssignNew(
      PromptTextBox,
      SMultiLineEditableTextBox
      ).AllowContextMenu(true)
       .Padding(10.0f)
       .HintText(
         FText::FromString(
           TEXT("Enter your prompt here...\nThis is mandatory!")
           )
         )
       .OnTextChanged(this, &SSkyboxAiWidget::OnPromptTextChanged)
       .OnTextCommitted(this, &SSkyboxAiWidget::OnPromTextCommitted)
       .AutoWrapText(true)
       .WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
       .Text(WidgetData.Prompt)
  ];

  AddExportTypeSelector(HBox);

  RootWidget->AddSlot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            .Padding(10.0f)
            .FillHeight(1.0f)
    [HBox.ToSharedRef()];
}

void SSkyboxAiWidget::OnPromptTextChanged(const FText &NewText)
{
  const int MaxTextLen = std::get<TUPLE_PROMPT_MAX_LEN_IDX>(WidgetData.Category);
  WidgetData.Prompt = FText::FromString(NewText.ToString().Mid(0, MaxTextLen));

  NotifyWidgetDataUpdated();
  UpdateTextCharacterCount(TEXT("Prompt"), PromptLabel, PromptTextBox, TUPLE_PROMPT_MAX_LEN_IDX);
}

void SSkyboxAiWidget::OnPromTextCommitted(const FText &NewText, ETextCommit::Type CommitType)
{
  WidgetData.Prompt = NewText;
  NotifyWidgetDataUpdated();
}

void SSkyboxAiWidget::AddExportTypeSelector(TSharedPtr<SHorizontalBox> RootWidget)
{
  RootWidget->AddSlot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Top)
            .Padding(10.0f)
            .FillWidth(0.35f)
  [
    SNew(SVerticalBox)

    + SVerticalBox::Slot().HAlign(HAlign_Fill)
                          .VAlign(VAlign_Top)
                          .FillHeight(0.2f)
                          .AutoHeight()
                          .Padding(10.0f)
    [
      SNew(SSearchBox).HintText(FText::FromString(TEXT("Select export type...")))
                      .MinDesiredWidth(200.0f)
                      .OnTextChanged(this, &SSkyboxAiWidget::OnExportTypeSearchTextChanged)
    ]

    + SVerticalBox::Slot().HAlign(HAlign_Fill)
                          .VAlign(VAlign_Top)
                          .FillHeight(0.8f)
                          .Padding(10.0f)
    [
      SAssignNew(
        ExportTypeListView,
        SListView<TSharedPtr<FText>>
        ).ItemHeight(24)
         .ListItemsSource(&FilteredExportTypes)
         .SelectionMode(ESelectionMode::Single)
         .OnGenerateRow(this, &SSkyboxAiWidget::OnExportTypeGenerateRow)
         .OnSelectionChanged(this, &SSkyboxAiWidget::OnExportTypeSelected)
    ]
  ];
}

void SSkyboxAiWidget::OnExportTypeSearchTextChanged(const FText &NewText)
{
  FilteredExportTypes.Empty();

  const FText SearchString = NewText;

  for (auto &[Key, Value] : ExportTypes)
  {
    if (Value.Contains(SearchString.ToString()))
    {
      FilteredExportTypes.Add(MakeShared<FText>(FText::FromString(Value)));
    }
  }

  UpdateListViewSelection<FString, FSkyboxAiExportTypesTuple>(
    ExportTypeListView,
    ExportTypes,
    FilteredExportTypes,
    WidgetData.ExportType,
    [this](const FString &Item)
    {
      return FText::FromString(Item);
    }
    );
}

TSharedRef<ITableRow> SSkyboxAiWidget::OnExportTypeGenerateRow(
  TSharedPtr<FText> Item,
  const TSharedRef<STableViewBase> &OwnerTable)
{
  return SNew(STableRow<TSharedPtr<FText>>, OwnerTable)
  [
    SNew(STextBlock).Text(*Item)
  ];
}

void SSkyboxAiWidget::OnExportTypeSelected(TSharedPtr<FText> InItem, ESelectInfo::Type SelectInfo)
{
  if (SelectInfo == ESelectInfo::Direct) return;

  FString NewValue = InItem.IsValid() ? InItem->ToString() : TEXT("");
  const int *ValueId = ExportTypes.FindKey(NewValue);
  const int FinalId = ValueId == nullptr ? -1 : *ValueId;

  if (FinalId == -1) return;

  WidgetData.ExportType = std::make_tuple(FinalId, NewValue);
  NotifyWidgetDataUpdated();
}

void SSkyboxAiWidget::AddNegativeTextAndCategories(TSharedPtr<SVerticalBox> RootWidget)
{
  const FText HintText = FText::FromString(
    TEXT("Enter your negative text here...\nOptional, leave blank if not needed")
    );

  TSharedPtr<SHorizontalBox> HBox = SNew(SHorizontalBox);

  HBox->AddSlot()
      .HAlign(HAlign_Left)
      .VAlign(VAlign_Center)
      .Padding(10.0f)
      .AutoWidth()
  [
    SAssignNew(
      NegativeTextLabel,
      STextBlock
      ).MinDesiredWidth(125)
       .Text(
         FText::FromString(
           FString::Printf(
             TEXT("Negative Text\r\n[0 / %d]"),
             std::get<TUPLE_NEGATIVE_TEXT_MAX_LEN_IDX>(WidgetData.Category)
             )
           )
         )
  ];

  HBox->AddSlot()
      .HAlign(HAlign_Fill)
      .VAlign(VAlign_Fill)
      .Padding(10.0f)
      .FillWidth(1.0)
  [
    SNew(SBox)
    [
      SAssignNew(
        NegativeTextTextBox,
        SMultiLineEditableTextBox
        ).AllowContextMenu(true)
         .Padding(10.0f)
         .HintText(HintText)
         .OnTextChanged(this, &SSkyboxAiWidget::OnNegativeTextChanged)
         .OnTextCommitted(this, &SSkyboxAiWidget::OnNegativeTextCommitted)
         .AutoWrapText(true)
         .WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
         .Text(WidgetData.NegativeText)
    ]
  ];

  AddCategorySelector(HBox);

  RootWidget->AddSlot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            .Padding(10.0f)
            .FillHeight(1.0f)
    [HBox.ToSharedRef()];
}

void SSkyboxAiWidget::OnNegativeTextChanged(const FText &NewText)
{
  const int MaxTextLen = std::get<TUPLE_NEGATIVE_TEXT_MAX_LEN_IDX>(WidgetData.Category);;
  WidgetData.NegativeText = FText::FromString(NewText.ToString().Mid(0, MaxTextLen));

  NotifyWidgetDataUpdated();
  UpdateTextCharacterCount(
    TEXT("Negative Text"),
    NegativeTextLabel,
    NegativeTextTextBox,
    TUPLE_NEGATIVE_TEXT_MAX_LEN_IDX
    );
}

void SSkyboxAiWidget::OnNegativeTextCommitted(const FText &NewText, ETextCommit::Type CommitType)
{
  WidgetData.NegativeText = NewText;
  NotifyWidgetDataUpdated();
}

void SSkyboxAiWidget::AddCategorySelector(TSharedPtr<SHorizontalBox> RootWidget)
{
  RootWidget->AddSlot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Top)
            .Padding(10.0f)
            .FillWidth(0.35f)
  [
    SNew(SVerticalBox)

    + SVerticalBox::Slot().HAlign(HAlign_Fill)
                          .VAlign(VAlign_Top)
                          .FillHeight(0.2f)
                          .AutoHeight()
                          .Padding(10.0f)
    [
      SNew(SSearchBox).HintText(FText::FromString(TEXT("Select categories...")))
                      .MinDesiredWidth(200.0f)
                      .OnTextChanged(this, &SSkyboxAiWidget::OnCategorySearchTextChanged)
    ]

    + SVerticalBox::Slot().HAlign(HAlign_Fill)
                          .VAlign(VAlign_Top)
                          .FillHeight(0.8f)
                          .Padding(10.0f)
    [
      SAssignNew(
        CategoryListView,
        SListView<TSharedPtr<FText>>
        ).ItemHeight(24)
         .ListItemsSource(&FilteredCategories)
         .SelectionMode(ESelectionMode::SingleToggle)
         .OnGenerateRow(this, &SSkyboxAiWidget::OnCategoryGenerateRow)
         .OnSelectionChanged(this, &SSkyboxAiWidget::OnCategorySelected)
    ]
  ];
}

void SSkyboxAiWidget::OnCategorySearchTextChanged(const FText &NewText)
{
  FilteredCategories.Empty();

  const FText SearchString = NewText;

  for (auto &[Key, Value] : Categories)
  {
    if (Value.Name.Contains(SearchString.ToString()))
    {
      FilteredCategories.Add(MakeShared<FText>(FText::FromString(Value.Name)));
    }
  }

  UpdateListViewSelection<FSkyboxListEntry, FSkyboxAiStylesTuple>(
    CategoryListView,
    Categories,
    FilteredCategories,
    WidgetData.Category,
    [this](const FSkyboxListEntry &Item)
    {
      return FText::FromString(Item.Name);
    }
    );
}

TSharedRef<ITableRow> SSkyboxAiWidget::OnCategoryGenerateRow(
  TSharedPtr<FText> Item,
  const TSharedRef<STableViewBase> &OwnerTable)
{
  return SNew(STableRow<TSharedPtr<FText>>, OwnerTable)
  [
    SNew(STextBlock).Text(*Item)
  ];
}

void SSkyboxAiWidget::OnCategorySelected(TSharedPtr<FText> InItem, ESelectInfo::Type SelectInfo)
{
  if (SelectInfo == ESelectInfo::Direct) return;

  const FString NewValue = InItem.IsValid() ? InItem->ToString() : TEXT("");
  const int *ValueId = Categories.FindKey(FSkyboxListEntry(NewValue));
  const int FinalId = ValueId == nullptr ? -1 : *ValueId;

  if (FinalId == -1) return;

  const auto [Name, PromptMaxLen, NegativeTextMaxLen] = Categories[FinalId];
  WidgetData.Category = std::make_tuple(FinalId, NewValue, PromptMaxLen, NegativeTextMaxLen);

  UpdateTextCharacterCount(TEXT("Prompt"), PromptLabel, PromptTextBox, TUPLE_PROMPT_MAX_LEN_IDX);
  UpdateTextCharacterCount(
    TEXT("Negative Text"),
    NegativeTextLabel,
    NegativeTextTextBox,
    TUPLE_NEGATIVE_TEXT_MAX_LEN_IDX
    );

  NotifyWidgetDataUpdated();
}

void SSkyboxAiWidget::AddBottomRow(TSharedPtr<SVerticalBox> RootWidget)
{
  RootWidget->AddSlot()
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Fill)
            .AutoHeight()
            .Padding(40.0f, 20.0f)
  [
    SNew(SHorizontalBox)

    + SHorizontalBox::Slot().HAlign(HAlign_Center)
                            .VAlign(VAlign_Center)
                            .AutoWidth()
                            .Padding(40.0f, 10.0f)
    [
      SAssignNew(RefreshListsButton, SButton).Text(FText::FromString(TEXT("Refresh Lists")))
                                             .OnClicked(this, &SSkyboxAiWidget::OnRefreshLists)
    ]

    + SHorizontalBox::Slot().HAlign(HAlign_Center)
                            .VAlign(VAlign_Center)
                            .AutoWidth()
                            .Padding(40.0f, 10.0f)
    [
      SAssignNew(GenerateButton, SButton).Text(FText::FromString(TEXT("Generate HDRI")))
                                         .OnClicked(this, &SSkyboxAiWidget::OnGenerateClicked)
    ]

    + SHorizontalBox::Slot().HAlign(HAlign_Center)
                            .VAlign(VAlign_Center)
                            .AutoWidth()
                            .Padding(0.0f, 10.0f)
    [
      SAssignNew(
        EnhancePromptCheckbox,
        SCheckBox
        ).ToolTipText(
           FText::FromString(TEXT("Enable enriching prompt with AI"))
           )
         .OnCheckStateChanged(this, &SSkyboxAiWidget::OnEnrichPromptChanged)
         .IsChecked(
           WidgetData.bEnrichPrompt ?
           ECheckBoxState::Checked :
           ECheckBoxState::Unchecked
           )
         .IsEnabled(PluginSettings->bEnablePremiumContent)
    ]

    + SHorizontalBox::Slot().HAlign(HAlign_Center)
                            .VAlign(VAlign_Center)
                            .AutoWidth()
                            .Padding(10.0f)
    [
      SNew(STextBlock).Text(FText::FromString(TEXT("Enrich Prompt")))
    ]
  ];
}

void SSkyboxAiWidget::OnEnrichPromptChanged(ECheckBoxState NewState)
{
  WidgetData.bEnrichPrompt = (NewState == ECheckBoxState::Checked);
  NotifyWidgetDataUpdated();
}

FReply SSkyboxAiWidget::OnGenerateClicked()
{
  if (!ValidateGenerateData()) return FReply::Handled();

  if (GenerateButton.IsValid()) GenerateButton->SetEnabled(false);

  ShowMessage(
    GenerateSkyboxNotification,
    GenerateSkyboxNotificationTitle,
    FText::FromString(TEXT("Starting Skybox Generation...")),
    SNotificationItem::CS_Pending
    );

  const FString TrimmedNegativeText = WidgetData.NegativeText.ToString().TrimStartAndEnd();
  FSkyboxGenerateRequest PostRequest;
  PostRequest.prompt = WidgetData.Prompt.ToString().TrimStartAndEnd();
  PostRequest.enhance_prompt = WidgetData.bEnrichPrompt;

  if (TrimmedNegativeText.Len() > 0) PostRequest.negative_text = TrimmedNegativeText;
  if (CategoryListView->GetNumItemsSelected() == 1)
  {
    PostRequest.skybox_style_id = std::get<TUPLE_KEY_IDX>(WidgetData.Category);
  }

  SkyboxApi->Skybox()->Post(
    PostRequest,
    [this](FSkyboxGenerateResponse *Response, int StatusCode, bool bConnectedSuccessfully)
    {
      if (Response == nullptr || !bConnectedSuccessfully || StatusCode >= 300)
      {
        if (GenerateButton.IsValid()) GenerateButton->SetEnabled(true);
        ShowMessage(
          GenerateSkyboxNotification,
          GenerateSkyboxNotificationTitle,
          FText::FromString(TEXT("Failed generating Skybox, See Message Log")),
          SNotificationItem::CS_Fail
          );
      }
      else
      {
        if (Response->status == TEXT("error"))
        {
          if (GenerateButton.IsValid()) GenerateButton->SetEnabled(true);
          ShowMessage(
            GenerateSkyboxNotification,
            GenerateSkyboxNotificationTitle,
            FText::FromString(FString::Printf(TEXT("Generation failed: %s"), *Response->error_message)),
            SNotificationItem::CS_Fail
            );
        }
        else
        {
          StartPollingGenerationStatus(Response->obfuscated_id);
        }
      }
    }
    );

  return FReply::Handled();
}

bool SSkyboxAiWidget::ValidateGenerateData() const
{
  const int PromptMaxLen = std::get<TUPLE_PROMPT_MAX_LEN_IDX>(WidgetData.Category);
  const int NegativeTextMaxLen = std::get<TUPLE_NEGATIVE_TEXT_MAX_LEN_IDX>(WidgetData.Category);
  const bool bExportTypeSelected = ExportTypeListView.IsValid() && ExportTypeListView->GetNumItemsSelected() == 1;
  const bool bCategoriesSelected = CategoryListView.IsValid() && CategoryListView->GetNumItemsSelected() <= 1;

  bool bSuccess = true;
  TArray<FString> ErrorMessage;
  ErrorMessage.Add(TEXT("Some validation errors occured, please review them and re-generate:\r\n\r\n"));

  if (PromptTextBox->GetText().ToString().Len() == 0)
  {
    bSuccess = false;
    ErrorMessage.Add(TEXT("Prompt can't be empty"));
  }

  if (PromptTextBox->GetText().ToString().Len() > PromptMaxLen)
  {
    bSuccess = false;
    ErrorMessage.Add(FString::Printf(TEXT("Prompt text is too long, max length is %d"), PromptMaxLen));
  }

  if (
    NegativeTextTextBox->GetText().ToString().Len() > 0 &&
    NegativeTextTextBox->GetText().ToString().Len() > NegativeTextMaxLen
  )
  {
    bSuccess = false;
    ErrorMessage.Add(FString::Printf(TEXT("Negative text is too long, max length is %d"), NegativeTextMaxLen));
  }

  if (!bExportTypeSelected)
  {
    bSuccess = false;
    ErrorMessage.Add(TEXT("An export type must be selected"));
  }

  if (!bCategoriesSelected)
  {
    bSuccess = false;
    ErrorMessage.Add(TEXT("You can only select up to one style"));
  }

  if (!bSuccess)
  {
    FMessageDialog::Open(
      EAppMsgCategory::Error,
      EAppMsgType::Ok,
      FText::FromString(FString::Join(ErrorMessage, TEXT("\r\n"))),
      FText::FromString(TEXT("Failed validating generate data"))
      );
  }

  return bSuccess;
}

void SSkyboxAiWidget::StartPollingGenerationStatus(const FString &SkyboxId)
{
  ShowMessage(
    GenerateSkyboxNotification,
    GenerateSkyboxNotificationTitle,
    FText::FromString(TEXT("Waiting for Skybox to be generated...")),
    SNotificationItem::CS_Pending
    );

  bGeneratePolling.Store(true);

  Async(
    EAsyncExecution::TaskGraph,
    [this, SkyboxId]()
    {
      while (bGeneratePolling.Load() && !IsEngineExitRequested())
      {
        PollGenerationStatus(SkyboxId);
        FPlatformProcess::Sleep(GetMutableDefault<UBlockadeLabsSkyboxAiSettings>()->ApiPollingInterval);
      }
    }
    );
}

void SSkyboxAiWidget::PollGenerationStatus(const FString &SkyboxId)
{
  SkyboxApi->Imagine()->GetRequestsObfuscatedId(
    SkyboxId,
    [this, SkyboxId](FImagineGetExportsResponse *Response, int StatusCode, bool bConnectedSuccessfully)
    {
      bool bStopPolling = false;
      bool bSuccess = true;
      FString ErrorMsg;

      if (Response == nullptr || !bConnectedSuccessfully || StatusCode >= 300)
      {
        bStopPolling = true;
        bSuccess = false;
        ErrorMsg = TEXT("Failed generating Skybox, See Message Log");
      }

      const FString Status = Response == nullptr ? TEXT("") : Response->request.status;

      if (bSuccess && Status == TEXT("error"))
      {
        bStopPolling = true;
        bSuccess = false;
        ErrorMsg = FString::Printf(TEXT("Generation failed: %s %s"), *SkyboxId, *Response->request.error_message);
      }

      if (bSuccess && Status == TEXT("abort"))
      {
        bStopPolling = true;
        bSuccess = false;
        ErrorMsg = FString::Printf(TEXT("Generation aborted: %s"), *SkyboxId);
      }

      if (bSuccess && Status == TEXT("complete"))
      {
        bStopPolling = true;
        bSuccess = true;
      }

      if (bStopPolling)
      {
        bGeneratePolling.Store(false);

        if (!bSuccess)
        {
          if (GenerateButton.IsValid()) GenerateButton->SetEnabled(true);
          ShowMessage(
            GenerateSkyboxNotification,
            GenerateSkyboxNotificationTitle,
            FText::FromString(ErrorMsg),
            SNotificationItem::CS_Fail
            );
        }
        else
        {
          ShowMessage(
            GenerateSkyboxNotification,
            GenerateSkyboxNotificationTitle,
            FText::FromString(FString::Printf(TEXT("Starting export of generated Skybox: %s"), *SkyboxId)),
            SNotificationItem::CS_Pending
            );

          FSkyboxExportRequest PostRequest;
          PostRequest.skybox_id = SkyboxId;
          PostRequest.type_id = std::get<TUPLE_KEY_IDX>(WidgetData.ExportType);

          SkyboxApi->Skybox()->PostExport(
            PostRequest,
            [this, SkyboxId](FSkyboxExportResponse *Response, int StatusCode, bool bConnectedSuccessfully)
            {
              if (Response == nullptr || !bConnectedSuccessfully || StatusCode >= 300)
              {
                if (GenerateButton.IsValid()) GenerateButton->SetEnabled(true);
                return ShowMessage(
                  GenerateSkyboxNotification,
                  GenerateSkyboxNotificationTitle,
                  FText::FromString(TEXT("Failed exporting Skybox, See Message Log")),
                  SNotificationItem::CS_Fail
                  );
              }

              if (Response->status == TEXT("error"))
              {
                if (GenerateButton.IsValid()) GenerateButton->SetEnabled(true);
                ShowMessage(
                  GenerateSkyboxNotification,
                  GenerateSkyboxNotificationTitle,
                  FText::FromString(FString::Printf(TEXT("Export failed: %s"), *Response->error_message)),
                  SNotificationItem::CS_Fail
                  );
              }
              else
              {
                StartPollingExportStatus(Response->id);
              }
            }
            );
        }
      }
      else
      {
        UE_LOG(SkyboxAiWidget, Log, TEXT("Generate status: %s"), *Status);
      }
    }
    );
}

void SSkyboxAiWidget::StartPollingExportStatus(const FString &SkyboxId)
{
  ShowMessage(
    GenerateSkyboxNotification,
    GenerateSkyboxNotificationTitle,
    FText::FromString(TEXT("Waiting for Skybox to be exported...")),
    SNotificationItem::CS_Pending
    );

  bExportPolling.Store(true);

  Async(
    EAsyncExecution::TaskGraph,
    [this, SkyboxId]()
    {
      while (bExportPolling.Load() && !IsEngineExitRequested())
      {
        PollExportStatus(SkyboxId);
        FPlatformProcess::Sleep(GetMutableDefault<UBlockadeLabsSkyboxAiSettings>()->ApiPollingInterval);
      }
    }
    );
}

void SSkyboxAiWidget::PollExportStatus(const FString &SkyboxId)
{
  SkyboxApi->Skybox()->GetExport(
    SkyboxId,
    [this, SkyboxId](FSkyboxExportResponse *Response, int StatusCode, bool bConnectedSuccessfully)
    {
      bool bStopPolling = false;
      bool bSuccess = true;
      FString ErrorMsg;

      if (Response == nullptr || !bConnectedSuccessfully || StatusCode >= 300)
      {
        bStopPolling = true;
        bSuccess = false;
        ErrorMsg = TEXT("Failed exporting Skybox, See Message Log");
      }

      const FString Status = Response == nullptr ? TEXT("") : Response->status;

      if (bSuccess && Status == TEXT("error"))
      {
        bStopPolling = true;
        bSuccess = false;
        ErrorMsg = FString::Printf(TEXT("Export failed: %s %s"), *SkyboxId, *Response->error_message);
      }

      if (bSuccess && Status == TEXT("abort"))
      {
        bStopPolling = true;
        bSuccess = false;
        ErrorMsg = FString::Printf(TEXT("Export aborted: %s"), *SkyboxId);
      }

      if (bSuccess && Status == TEXT("complete"))
      {
        bStopPolling = true;
        bSuccess = true;
      }

      if (bStopPolling)
      {
        bExportPolling.Store(false);

        if (!bSuccess)
        {
          ShowMessage(
            GenerateSkyboxNotification,
            GenerateSkyboxNotificationTitle,
            FText::FromString(ErrorMsg),
            SNotificationItem::CS_Fail
            );
        }
        else
        {
          ShowMessage(
            GenerateSkyboxNotification,
            GenerateSkyboxNotificationTitle,
            FText::FromString(FString::Printf(TEXT("Downloading Skybox: %s"), *SkyboxId)),
            SNotificationItem::CS_Pending
            );

          SkyboxApi->SaveExportedImage(
            Response->file_url,
            [this, SkyboxId](bool bSuccess)
            {
              if (GenerateButton.IsValid()) GenerateButton->SetEnabled(true);
              ShowMessage(
                GenerateSkyboxNotification,
                GenerateSkyboxNotificationTitle,
                FText::FromString(FString::Printf(TEXT("Finished downloading Skybox: %s"), *SkyboxId)),
                bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail
                );
            }
            );
        }
      }
      else
      {
        UE_LOG(SkyboxAiWidget, Log, TEXT("Export status: %s"), *Status);
      }
    }
    );
}

FReply SSkyboxAiWidget::OnRefreshLists()
{
  Categories.Empty();
  FilteredCategories.Empty();

  ExportTypes.Empty();
  FilteredExportTypes.Empty();

  if (CategoryListView.IsValid()) CategoryListView->RequestListRefresh();
  if (ExportTypeListView.IsValid()) ExportTypeListView->RequestListRefresh();

  ExecuteRefreshListAsync();

  return FReply::Handled();
}

void SSkyboxAiWidget::ExecuteRefreshListAsync()
{
  if (RefreshListsButton.IsValid()) RefreshListsButton->SetEnabled(false);

  ShowMessage(
    RefreshListsNotification,
    RefreshListsNotificationTitle,
    FText::FromString(TEXT("Refreshing Lists...")),
    SNotificationItem::CS_Pending
    );

  TSharedPtr<int32> CompletionCounter = MakeShared<int32>(2);
  TSharedPtr<int32> FailureCounter = MakeShared<int32>(0);

  auto OnApiCallComplete = [this, CompletionCounter, FailureCounter](bool bFailed)
  {
    if (bFailed) FPlatformAtomics::InterlockedIncrement(FailureCounter.Get());
    if (FPlatformAtomics::InterlockedDecrement(CompletionCounter.Get()) != 0) return;

    Async(
      EAsyncExecution::TaskGraphMainThread,
      [this, FailureCounter]()
      {
        const bool bOk = *FailureCounter.Get() == 0;

        ShowMessage(
          RefreshListsNotification,
          RefreshListsNotificationTitle,
          bOk ?
          FText::FromString(TEXT("Completed Successfuly")) :
          FText::FromString(TEXT("Some items failed, See Message Log")),
          bOk ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail
          );

        if (RefreshListsButton.IsValid()) RefreshListsButton->SetEnabled(true);
        if (PromptLabel.IsValid())
        {
          UpdateTextCharacterCount(
            TEXT("Prompt"),
            PromptLabel,
            PromptTextBox,
            TUPLE_PROMPT_MAX_LEN_IDX
            );
        }
        if (NegativeTextLabel.IsValid())
        {
          UpdateTextCharacterCount(
            TEXT("Negative Text"),
            NegativeTextLabel,
            NegativeTextTextBox,
            TUPLE_NEGATIVE_TEXT_MAX_LEN_IDX
            );
        }
      }
      );
  };

  SkyboxApi.Get()->Skybox()->GetStyles(
    [this, OnApiCallComplete](FSkyboxAiStyles &Styles, int StatusCode, bool bConnectedSuccessfully)
    {
      if (StatusCode >= 300 || !bConnectedSuccessfully) return OnApiCallComplete(true);

      LoadCategoriesFromList(Styles);
      OnApiCallComplete(false);
    }
    );

  SkyboxApi.Get()->Skybox()->GetExports(
    [this, OnApiCallComplete](FSkyboxAiExportTypes &Types, int StatusCode, bool bConnectedSuccessfully)
    {
      if (StatusCode >= 300 || !bConnectedSuccessfully) return OnApiCallComplete(true);

      LoadExportTypesFromList(Types);
      OnApiCallComplete(false);
    }
    );
}

void SSkyboxAiWidget::UpdateTextCharacterCount(
  const FString &LabelTitle,
  const TSharedPtr<STextBlock> &InPromptLabel,
  const TSharedPtr<SMultiLineEditableTextBox> &InPromptTextBox,
  const int TupleLenIdx) const
{
  const int MaxTextLen = TupleLenIdx == TUPLE_PROMPT_MAX_LEN_IDX ?
    std::get<TUPLE_PROMPT_MAX_LEN_IDX>(WidgetData.Category) :
    std::get<TUPLE_NEGATIVE_TEXT_MAX_LEN_IDX>(WidgetData.Category);

  InPromptLabel->SetText(
    FText::FromString(
      FString::Printf(
        TEXT("%s\r\n[%d / %d]"),
        *LabelTitle,
        InPromptTextBox->GetText().ToString().Len(),
        MaxTextLen
        )
      )
    );

  const FSlateColor Color = InPromptTextBox->GetText().ToString().Len() > MaxTextLen ?
    FSlateColor(FLinearColor(1.0f, 0.0f, 0.0f)) :
    FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f));

  NegativeTextTextBox->SetForegroundColor(Color);
}

void SSkyboxAiWidget::ShowMessage(
  TSharedPtr<SNotificationItem> &Notification,
  const FText &Title,
  const FText &Message,
  const SNotificationItem::ECompletionState State)
{
  if (!Notification.IsValid())
  {
    auto InvokeMessageLog = []()
    {
      FGlobalTabmanager::Get()->TryInvokeTab(FName("MessageLog"));
    };

    FNotificationInfo Info(FText::FromString(TEXT("")));

    Info.bUseLargeFont = true;
    Info.bFireAndForget = false;
    Info.FadeOutDuration = 1.0f;
    Info.ExpireDuration = 5.0f;
    Info.bUseThrobber = true;
    Info.ForWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
    Info.Hyperlink = FSimpleDelegate::CreateLambda(InvokeMessageLog);
    Info.HyperlinkText = FText::FromString(TEXT("View Message Log"));

    Notification = FSlateNotificationManager::Get().AddNotification(Info);
  }

  if (Notification.IsValid())
  {
    Notification->SetText(Title);
    Notification->SetSubText(Message);

    if (Notification->GetCompletionState() != State) Notification->SetCompletionState(State);
    if (State != SNotificationItem::CS_Pending)
    {
      Notification->ExpireAndFadeout();
      Notification.Reset();
    }
  }
}

void SSkyboxAiWidget::NotifyWidgetDataUpdated() const
{
  if (OnSkyboxAiWidgetDataChanged.ExecuteIfBound(WidgetData)) return;
  UE_LOG(SkyboxAiWidget, Warning, TEXT("Failed to execute OnSkyboxAiWidgetDataChanged"));
}
