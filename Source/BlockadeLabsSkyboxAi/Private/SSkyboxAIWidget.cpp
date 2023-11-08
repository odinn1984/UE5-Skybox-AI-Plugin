#include "SSkyboxAiWidget.h"
#include "Framework/Notifications/NotificationManager.h"
#include "SkyboxAi/SKyboxAiHttpClient.h"
#include "SkyboxAi/SkyboxProvider.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"

DEFINE_LOG_CATEGORY(SkyboxAiWidget);

void SSkyboxAiWidget::Construct(const FArguments &InArgs)
{
  if (!SkyboxApi.IsValid()) SkyboxApi = NewObject<USkyboxApi>();

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

void SSkyboxAiWidget::LoadCategoriesFromList(const TMap<int, FString> &List)
{
  LoadViewListFromMap(Categories, FilteredCategories, CategoryListView, List, WidgetData.Category, TEXT("Categories"));
}

void SSkyboxAiWidget::LoadExportTypesFromList(const TMap<int, FString> &List)
{
  LoadViewListFromMap(
    ExportTypes,
    FilteredExportTypes,
    ExportTypeListView,
    List,
    WidgetData.ExportType,
    TEXT("Export Types")
    );
}

void SSkyboxAiWidget::LoadViewListFromMap(
  TMap<int, FString> &OutValues,
  TArray<TSharedPtr<FText>> &OutFilteredValues,
  const FSkyboxAiWidgetListView &OutListView,
  const TMap<int, FString> &InList,
  FSkyboxAiWidgetTuple &CurrentValue,
  const FString &InListSource)
{
  for (auto &Item : InList)
  {
    OutValues.Add(Item.Key, Item.Value);
    OutFilteredValues.Add(MakeShared<FText>(FText::FromString(Item.Value)));
  }

  if (!OutListView.IsValid())
  {
    return ShowMessage(
      RefreshListsNotification,
      RefreshListsNotificationTitle,
      FText::FromString(FString::Printf(TEXT("[%s] Invalid List View Provided"), *InListSource)),
      SNotificationItem::CS_Fail
      );
  }

  UpdateListViewSelection(OutListView, OutValues, OutFilteredValues, CurrentValue);
}

void SSkyboxAiWidget::UpdateListViewSelection(
  const FSkyboxAiWidgetListView &ListView,
  TMap<int, FString> &Map,
  TArray<TSharedPtr<FText>> &List,
  FSkyboxAiWidgetTuple &CurrentValue) const
{
  const FString Value = std::get<TUPLE_VALUE_IDX>(CurrentValue);
  const TFunction<bool(const TSharedPtr<FText> &, const FString &)> Equals = [](
    const TSharedPtr<FText> &Item,
    const FString &Value)
  {
    return Item->EqualTo(FText::FromString(Value));
  };

  if (Value.IsEmpty() || (List.Num() > 0 && ListContains<TSharedPtr<FText>, FString>(
    List,
    Value,
    Equals
    )))
  {
    if (const int ItemIndex = FindInList<TSharedPtr<FText>, FString>(List, Value, Equals); ItemIndex != INDEX_NONE)
    {
      ListView->SetSelection(List[ItemIndex]);
    }
  }
  else if (List.Num() > 0)
  {
    const int *DefaultValueId = Map.FindKey(List[0]->ToString());
    int DefaultId = DefaultValueId == nullptr ? -1 : *DefaultValueId;

    CurrentValue = std::make_tuple(DefaultId, List[0]->ToString());
    ListView->SetSelection(List[0]);
    NotifyWidgetDataUpdated();
  }

  if (ListView.IsValid()) ListView->RequestListRefresh();
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
    SNew(STextBlock).MinDesiredWidth(125)
                    .Text(FText::FromString(TEXT("Prompt")))
  ];

  HBox->AddSlot()
      .HAlign(HAlign_Fill)
      .VAlign(VAlign_Fill)
      .Padding(10.0f)
      .FillWidth(1.0)
  [
    SNew(SMultiLineEditableTextBox).AllowContextMenu(true)
                                   .Padding(10.0f)
                                   .HintText(FText::FromString(TEXT("Enter your prompt here...\nThis is mandatory!")))
                                   .OnTextChanged(this, &SSkyboxAiWidget::OnPromptTextChanged)
                                   .OnTextCommitted(this, &SSkyboxAiWidget::OnPromTextCommitted)
                                   .AutoWrapText(true)
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
  WidgetData.Prompt = NewText;
  NotifyWidgetDataUpdated();
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
         .SelectionMode(ESelectionMode::SingleToggle)
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

  UpdateListViewSelection(ExportTypeListView, ExportTypes, FilteredExportTypes, WidgetData.ExportType);
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
  int FinalId = ValueId == nullptr ? -1 : *ValueId;

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
    SNew(STextBlock).MinDesiredWidth(125)
                    .Text(FText::FromString(TEXT("Negative Text")))
  ];

  HBox->AddSlot()
      .HAlign(HAlign_Fill)
      .VAlign(VAlign_Fill)
      .Padding(10.0f)
      .FillWidth(1.0)
  [
    SNew(SBox)
    [
      SNew(SMultiLineEditableTextBox).AllowContextMenu(true)
                                     .Padding(10.0f)
                                     .HintText(HintText)
                                     .OnTextChanged(this, &SSkyboxAiWidget::OnNegativeTextChanged)
                                     .OnTextCommitted(this, &SSkyboxAiWidget::OnNegativeTextCommitted)
                                     .AutoWrapText(true)
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
  WidgetData.NegativeText = NewText;
  NotifyWidgetDataUpdated();
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
    if (Value.Contains(SearchString.ToString()))
    {
      FilteredCategories.Add(MakeShared<FText>(FText::FromString(Value)));
    }
  }

  UpdateListViewSelection(CategoryListView, Categories, FilteredCategories, WidgetData.Category);
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

  FString NewValue = InItem.IsValid() ? InItem->ToString() : TEXT("");
  const int *ValueId = Categories.FindKey(NewValue);
  int FinalId = ValueId == nullptr ? -1 : *ValueId;

  WidgetData.Category = std::make_tuple(FinalId, NewValue);

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
      SNew(SCheckBox).ToolTipText(FText::FromString(TEXT("Enable enriching prompt with AI")))
                     .OnCheckStateChanged(this, &SSkyboxAiWidget::OnEnrichPromptChanged)
                     .IsChecked(WidgetData.bEnrichPrompt ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
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
  if (GenerateButton.IsValid()) GenerateButton->SetEnabled(false);

  ShowMessage(
    GenerateSkyboxNotification,
    GenerateSkyboxNotificationTitle,
    FText::FromString(TEXT("Generating Skybox...")),
    SNotificationItem::CS_Pending
    );

  // Start generate API call
  // When API call is done, start polling for generate status
  // when status is success start download of the image
  // when done update message with success
  // if failure happens update message with failure
  // should probably also save the download URL on disk so that if something goes wrong the user can download it
  // either failure or success, re-enable the generate button

  FSkyboxGenerateRequest PostRequest;
  PostRequest.prompt = WidgetData.Prompt.ToString();
  PostRequest.negative_text = WidgetData.NegativeText.ToString();
  PostRequest.enhance_prompt = WidgetData.bEnrichPrompt;
  PostRequest.skybox_style_id = std::get<TUPLE_KEY_IDX>(WidgetData.Category);

  SkyboxApi->Skybox()->Post(PostRequest, [this](FSkyboxGenerateResponse *Response, int StatusCode, bool bConnectedSuccessfully)
  {
    if (Response == nullptr || !bConnectedSuccessfully || StatusCode >= 300)
    {
      return;
    }

  });


  return FReply::Handled();
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
