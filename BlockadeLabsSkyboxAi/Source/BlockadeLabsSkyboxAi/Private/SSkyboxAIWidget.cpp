#include "SSkyboxAiWidget.h"
#include "BlockadeLabsSkyboxAiSettings.h"
#include "CustomMessageDialog.h"
#include "ImagineProvider.h"
#include "InputDialogWidget.h"
#include "Framework/Notifications/NotificationManager.h"
#include "SkyboxAiApi.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Async/Async.h"

DEFINE_LOG_CATEGORY(SkyboxAiWidget);

void SSkyboxAiWidget::Construct(const FArguments& InArgs)
{
	if (!SkyboxApi.IsValid()) SkyboxApi = NewObject<USkyboxAiApi>();

	if (!PluginSettings.IsValid())
	{
		PluginSettings = NewObject<UBlockadeLabsSkyboxAiSettings>();
		PluginSettings->OnSettingChanged().AddLambda(
			[this](UObject* Object, struct FPropertyChangedEvent& Event)
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
	bStartingUp = false;
}

void SSkyboxAiWidget::LoadCategoriesFromList(const FSkyboxAiStyles& List)
{
	LoadViewListFromMap<FSkyboxListEntry, FSkyboxAiStylesTuple>(
		Categories,
		FilteredCategories,
		CategoryListView,
		List,
		WidgetData.Category,
		false,
		[this](const FSkyboxListEntry& Item)
		{
			return FText::FromString(Item.Name);
		}
	);
}

void SSkyboxAiWidget::LoadExportTypesFromList(const FSkyboxAiExportTypes& List)
{
	LoadViewListFromMap<FString, FSkyboxAiExportTypesTuple>(
		ExportTypes,
		FilteredExportTypes,
		ExportTypeListView,
		List,
		WidgetData.ExportType,
		true,
		[this](const FString& Item)
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

void SSkyboxAiWidget::OnPromptTextChanged(const FText& NewText)
{
	const int MaxTextLen = std::get<TUPLE_PROMPT_MAX_LEN_IDX>(WidgetData.Category);
	WidgetData.Prompt = FText::FromString(NewText.ToString().Mid(0, MaxTextLen));

	NotifyWidgetDataUpdated();
	UpdateTextCharacterCount(TEXT("Prompt"), PromptLabel, PromptTextBox, TUPLE_PROMPT_MAX_LEN_IDX);
}

void SSkyboxAiWidget::OnPromTextCommitted(const FText& NewText, ETextCommit::Type CommitType)
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

void SSkyboxAiWidget::OnExportTypeSearchTextChanged(const FText& NewText)
{
	FilteredExportTypes.Empty();

	const FText SearchString = NewText;

	for (auto& [Key, Value] : ExportTypes)
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
		true,
		[this](const FString& Item)
		{
			return FText::FromString(Item);
		}
	);
}

TSharedRef<ITableRow> SSkyboxAiWidget::OnExportTypeGenerateRow(
	TSharedPtr<FText> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
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
	const int* ValueId = ExportTypes.FindKey(NewValue);
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

void SSkyboxAiWidget::OnNegativeTextChanged(const FText& NewText)
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

void SSkyboxAiWidget::OnNegativeTextCommitted(const FText& NewText, ETextCommit::Type CommitType)
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

void SSkyboxAiWidget::OnCategorySearchTextChanged(const FText& NewText)
{
	FilteredCategories.Empty();

	const FText SearchString = NewText;

	for (auto& [Key, Value] : Categories)
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
		false,
		[this](const FSkyboxListEntry& Item)
		{
			return FText::FromString(Item.Name);
		}
	);
}

TSharedRef<ITableRow> SSkyboxAiWidget::OnCategoryGenerateRow(
	TSharedPtr<FText> Item,
	const TSharedRef<STableViewBase>& OwnerTable)
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
	const int* ValueId = Categories.FindKey(FSkyboxListEntry(NewValue));
	const int FinalId = ValueId == nullptr ? -1 : *ValueId;

	if (FinalId == INDEX_NONE && NewValue.TrimStartAndEnd().Len() == 0)
	{
		WidgetData.Category = std::make_tuple(DEFAULT_ID, TEXT(""), DEFAULT_MAX_TEXT_LEN, DEFAULT_MAX_TEXT_LEN);
	}
	else
	{
		const auto [Name, ImageUrl, PromptMaxLen, NegativeTextMaxLen] = Categories[FinalId];
		WidgetData.Category = std::make_tuple(FinalId, NewValue, PromptMaxLen, NegativeTextMaxLen);
	}

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
			SAssignNew(GenerateButton, SButton).Text(FText::FromString(TEXT("Generate")))
			                                   .OnClicked(this, &SSkyboxAiWidget::OnGenerateClicked)
		]

		+ SHorizontalBox::Slot().HAlign(HAlign_Center)
		                        .VAlign(VAlign_Center)
		                        .AutoWidth()
		                        .Padding(40.0f, 10.0f)
		[
			SAssignNew(ImportButton, SButton).Text(FText::FromString(TEXT("Import...")))
			                                 .OnClicked(this, &SSkyboxAiWidget::OnImportClicked)
		]

		+ SHorizontalBox::Slot().HAlign(HAlign_Center)
		                        .VAlign(VAlign_Center)
		                        .AutoWidth()
		                        .Padding(40.0f, 10.0f)
		[
			SAssignNew(RemixButton, SButton).Text(FText::FromString(TEXT("Remix...")))
			                                .OnClicked(this, &SSkyboxAiWidget::OnRemixClicked)
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

	SetButtonsEnabled(false);
	ShowMessage(
		GenerateSkyboxNotification,
		GenerateSkyboxNotificationTitle,
		FText::FromString(TEXT("Starting Skybox Generation...")),
		SNotificationItem::CS_Pending
	);

	FSkyboxGenerateRequest Request = GetGenerateRequest();
	ExecuteGenerate(Request);

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
		FCustomMessageDialog::Open(
			EAppMsgCategory::Error,
			EAppMsgType::Ok,
			FText::FromString(FString::Join(ErrorMessage, TEXT("\r\n"))),
			SSkyboxAiWidget::FailedValidationTitle
		);
	}

	return bSuccess;
}

FSkyboxGenerateRequest SSkyboxAiWidget::GetGenerateRequest() const
{
	FSkyboxGenerateRequest PostRequest;
	const FString TrimmedNegativeText = WidgetData.NegativeText.ToString().TrimStartAndEnd();
	PostRequest.prompt = WidgetData.Prompt.ToString().TrimStartAndEnd();
	PostRequest.enhance_prompt = WidgetData.bEnrichPrompt;

	if (TrimmedNegativeText.Len() > 0) PostRequest.negative_text = TrimmedNegativeText;
	if (CategoryListView->GetNumItemsSelected() == 1)
	{
		PostRequest.skybox_style_id = std::get<TUPLE_KEY_IDX>(WidgetData.Category);
	}

	return PostRequest;
}

void SSkyboxAiWidget::ExecuteGenerate(FSkyboxGenerateRequest& Request)
{
	SkyboxApi->Skybox()->Post(
		Request,
		[this](FSkyboxGenerateResponse* Response, int StatusCode, bool bConnectedSuccessfully)
		{
			if (Response == nullptr || !bConnectedSuccessfully || StatusCode >= 300)
			{
				SetButtonsEnabled(true);
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
					SetButtonsEnabled(true);
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
}

void SSkyboxAiWidget::StartPollingGenerationStatus(const FString& SkyboxId)
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
			int PollCount = 0;
			while (bGeneratePolling.Load() && !IsEngineExitRequested())
			{
				PollGenerationStatus(SkyboxId, PollCount++);
				FPlatformProcess::Sleep(GetMutableDefault<UBlockadeLabsSkyboxAiSettings>()->ApiPollingInterval);
			}
		}
	);
}

void SSkyboxAiWidget::PollGenerationStatus(const FString& SkyboxId, int PollCount)
{
	SkyboxApi->Imagine()->GetRequestsObfuscatedId(
		SkyboxId,
		[this, SkyboxId, PollCount](FImagineGetExportsResponseRequest* Response, int StatusCode, bool bConnectedSuccessfully)
		{
			UBlockadeLabsSkyboxAiSettings* EditorSettings = GetMutableDefault<UBlockadeLabsSkyboxAiSettings>();
			bool bStopPolling = false;
			bool bSuccess = true;
			FString ErrorMsg;

			if (PollCount >= EditorSettings->MaxPollAttempts)
			{
				bStopPolling = true;
				bSuccess = false;
				ErrorMsg = TEXT("Timeout waiting for skybox to generate");
			}

			if (bSuccess && Response == nullptr || !bConnectedSuccessfully || StatusCode >= 300)
			{
				bStopPolling = true;
				bSuccess = false;
				ErrorMsg = TEXT("Failed generating Skybox, See Message Log");
			}

			const FString Status = Response == nullptr ? TEXT("") : Response->status;

			if (bSuccess && Status == TEXT("error"))
			{
				bStopPolling = true;
				bSuccess = false;
				ErrorMsg = FString::Printf(TEXT("Generation failed: %s %s"), *SkyboxId, *Response->error_message);
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
					SetButtonsEnabled(true);
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
						[this, SkyboxId](FSkyboxExportResponse* Response, int StatusCode, bool bConnectedSuccessfully)
						{
							if (Response == nullptr || !bConnectedSuccessfully || StatusCode >= 300)
							{
								SetButtonsEnabled(true);
								return ShowMessage(
									GenerateSkyboxNotification,
									GenerateSkyboxNotificationTitle,
									FText::FromString(TEXT("Failed exporting Skybox, See Message Log")),
									SNotificationItem::CS_Fail
								);
							}

							if (Response->status == TEXT("error"))
							{
								SetButtonsEnabled(true);
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

void SSkyboxAiWidget::StartPollingExportStatus(const FString& SkyboxId)
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
			int PollCount = 0;
			while (bExportPolling.Load() && !IsEngineExitRequested())
			{
				PollExportStatus(SkyboxId, PollCount++);
				FPlatformProcess::Sleep(GetMutableDefault<UBlockadeLabsSkyboxAiSettings>()->ApiPollingInterval);
			}
		}
	);
}

void SSkyboxAiWidget::PollExportStatus(const FString& SkyboxId, int PollCount)
{
	SkyboxApi->Skybox()->GetExportById(
		SkyboxId,
		[this, SkyboxId, PollCount](FSkyboxExportResponse* Response, int StatusCode, bool bConnectedSuccessfully)
		{
			UBlockadeLabsSkyboxAiSettings* EditorSettings = GetMutableDefault<UBlockadeLabsSkyboxAiSettings>();
			bool bStopPolling = false;
			bool bSuccess = true;
			FString ErrorMsg;

			if (PollCount >= EditorSettings->MaxPollAttempts)
			{
				bStopPolling = true;
				bSuccess = false;
				ErrorMsg = TEXT("Timeout waiting for export");
			}

			if (bSuccess && Response == nullptr || !bConnectedSuccessfully || StatusCode >= 300)
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
							SetButtonsEnabled(true);
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

FReply SSkyboxAiWidget::OnImportClicked()
{
	SetButtonsEnabled(false);
	ShowModal(SSkyboxAiWidget::ImportSkyboxNotificationTitle, [this](const uint32 Id) { ExecuteImport(Id); });

	return FReply::Handled();
}

void SSkyboxAiWidget::ExecuteImport(const uint32 SkyboxImagineId)
{
	SetButtonsEnabled(false);
	ShowMessage(
		ImportNotification,
		ImportSkyboxNotificationTitle,
		FText::FromString(TEXT("Importing Skybox...")),
		SNotificationItem::CS_Pending
	);

	SkyboxApi->Imagine()->GetRequests(
		SkyboxImagineId,
		[this](FImagineGetExportsResponseRequest* Response, int StatusCode, bool bConnectedSuccessfully)
		{
			if (Response == nullptr || !bConnectedSuccessfully || StatusCode >= 300)
			{
				ShowMessage(
					ImportNotification,
					ImportSkyboxNotificationTitle,
					FText::FromString(TEXT("Failed importing Skybox, See Message Log")),
					SNotificationItem::CS_Fail
				);
			}
			else if (Response->status == TEXT("error"))
			{
				ShowMessage(
					ImportNotification,
					ImportSkyboxNotificationTitle,
					FText::FromString(FString::Printf(TEXT("Import failed: %s"), *Response->error_message)),
					SNotificationItem::CS_Fail
				);
			}
			else
			{
				PromptTextBox->SetText(FText::FromString(Response->prompt));
				NegativeTextTextBox->SetText(FText::FromString(Response->negative_text));
				CategoryListView->ClearSelection();

				const int StyleIdx = Response->skybox_style_id;
				if (CategoryListView.IsValid() && StyleIdx != INDEX_NONE)
				{
					for (TSharedPtr<FText> Category : FilteredCategories)
					{
						FSkyboxListEntry CurrentCategory = Categories[StyleIdx];
						if (Category->ToString().Equals(CurrentCategory.Name)) CategoryListView->SetSelection(Category);
					}
				}

				ShowMessage(
					ImportNotification,
					ImportSkyboxNotificationTitle,
					FText::FromString(TEXT("Finished importing")),
					SNotificationItem::CS_Success
				);
			}

			SetButtonsEnabled(true);
		}
	);
}

FReply SSkyboxAiWidget::OnRemixClicked()
{
	if (!ValidateGenerateData()) return FReply::Handled();

	SetButtonsEnabled(false);
	ShowModal(SSkyboxAiWidget::RemixSkyboxNotificationTitle, [this](const uint32 Id) { ExecuteRemix(Id); });

	return FReply::Handled();
}

void SSkyboxAiWidget::ExecuteRemix(const uint32 SkyboxImagineId)
{
	SetButtonsEnabled(false);
	ShowMessage(
		GenerateSkyboxNotification,
		GenerateSkyboxNotificationTitle,
		FText::FromString(TEXT("Starting Skybox Generation...")),
		SNotificationItem::CS_Pending
	);

	FSkyboxGenerateRequest Request = GetGenerateRequest();
	Request.remix_imagine_id = SkyboxImagineId;

	ExecuteGenerate(Request);
}

FReply SSkyboxAiWidget::OnRefreshLists()
{
	if (!bStartingUp)
	{
		const auto Answer = FCustomMessageDialog::Open(
			EAppMsgCategory::Error,
			EAppMsgType::OkCancel,
			FText::FromString(TEXT("Refreshing lists will revert to default selection, are you sure?")),
			SSkyboxAiWidget::RefreshListConfirmationTitle
		);

		if (Answer == EAppReturnType::Cancel) return FReply::Handled();
	}

	Categories.Empty();
	FilteredCategories.Empty();

	ExportTypes.Empty();
	FilteredExportTypes.Empty();

	WidgetData.Category = std::make_tuple(DEFAULT_ID, TEXT(""), DEFAULT_MAX_TEXT_LEN, DEFAULT_MAX_TEXT_LEN);
	WidgetData.ExportType = std::make_tuple(DEFAULT_ID, TEXT(""));

	if (CategoryListView.IsValid())
	{
		CategoryListView->ClearSelection();
		CategoryListView->RequestListRefresh();
	}
	if (ExportTypeListView.IsValid())
	{
		ExportTypeListView->ClearSelection();
		ExportTypeListView->RequestListRefresh();
	}

	ExecuteRefreshList();

	return FReply::Handled();
}

void SSkyboxAiWidget::ExecuteRefreshList()
{
	SetButtonsEnabled(false);
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

				SetButtonsEnabled(true);
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
		[this, OnApiCallComplete](FSkyboxAiStyles& Styles, int StatusCode, bool bConnectedSuccessfully)
		{
			if (StatusCode >= 300 || !bConnectedSuccessfully) return OnApiCallComplete(true);

			LoadCategoriesFromList(Styles);
			OnApiCallComplete(false);
		}
	);

	SkyboxApi.Get()->Skybox()->GetExport(
		[this, OnApiCallComplete](FSkyboxAiExportTypes& Types, int StatusCode, bool bConnectedSuccessfully)
		{
			if (StatusCode >= 300 || !bConnectedSuccessfully) return OnApiCallComplete(true);

			LoadExportTypesFromList(Types);
			OnApiCallComplete(false);
		}
	);
}

void SSkyboxAiWidget::SetButtonsEnabled(const bool bEnabled) const
{
	if (RefreshListsButton.IsValid()) RefreshListsButton->SetEnabled(bEnabled);
	if (GenerateButton.IsValid()) GenerateButton->SetEnabled(bEnabled);
	if (RemixButton.IsValid()) RemixButton->SetEnabled(bEnabled);
	if (ImportButton.IsValid()) ImportButton->SetEnabled(bEnabled);
}

void SSkyboxAiWidget::UpdateTextCharacterCount(
	const FString& LabelTitle,
	const TSharedPtr<STextBlock>& InPromptLabel,
	const TSharedPtr<SMultiLineEditableTextBox>& InPromptTextBox,
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

	InPromptTextBox->SetForegroundColor(Color);
}

void SSkyboxAiWidget::ShowMessage(
	TSharedPtr<SNotificationItem>& Notification,
	const FText& Title,
	const FText& Message,
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

void SSkyboxAiWidget::ShowModal(const FText& Title, TFunction<void(const uint32)> OnConfirm)
{
	TSharedRef<SWindow> DialogWindow =
		SNew(SWindow).Title(Title)
		             .ClientSize(FVector2D(350, 100))
		             .SupportsMinimize(false)
		             .SupportsMaximize(false)
		             .SizingRule(ESizingRule::FixedSize)
		             .IsTopmostWindow(true);

	TSharedPtr<SInputDialogWidget> InputDialog =
		SNew(SInputDialogWidget).ParentWindow(DialogWindow)
		                        .OnConfirm_Lambda(OnConfirm)
		                        .OnCancel_Lambda([this]() { SetButtonsEnabled(true); });

	DialogWindow->SetContent(InputDialog.ToSharedRef());
	DialogWindow->SetAsModalWindow();
	DialogWindow->SetOnWindowClosed(
		FOnWindowClosed::CreateLambda([this](const TSharedRef<SWindow>& Window) { SetButtonsEnabled(true); })
	);

	FSlateApplication::Get().AddModalWindow(DialogWindow, nullptr);
}

void SSkyboxAiWidget::NotifyWidgetDataUpdated() const
{
	if (OnSkyboxAiWidgetDataChanged.ExecuteIfBound(WidgetData)) return;
	UE_LOG(SkyboxAiWidget, Warning, TEXT("Failed to execute OnSkyboxAiWidgetDataChanged"));
}
