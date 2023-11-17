#include "InputDialogWidget.h"

#include "CustomMessageDialog.h"

DEFINE_LOG_CATEGORY(InputDialogWidget);

void SInputDialogWidget::Construct(const FArguments& InArgs)
{
	ParentWindow = InArgs._ParentWindow;
	DialogTitle = InArgs._DialogTitle;
	OnConfirm = InArgs._OnConfirm;
	OnCancel = InArgs._OnCancel;

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot().Padding(10)
		                      .AutoHeight()
		[
			SAssignNew(InputField, SEditableTextBox)
		]

		+ SVerticalBox::Slot().Padding(10)
		                      .HAlign(HAlign_Center)
		                      .AutoHeight()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot().Padding(10)
			                        .AutoWidth()
			[
				SNew(SButton).Text(FText::FromString(TEXT("Confirm")))
				             .OnClicked_Lambda(
					             [this]() -> FReply
					             {
						             if (!IsInputValid())
						             {
							             FCustomMessageDialog::Open(
								             EAppMsgCategory::Error,
								             EAppMsgType::Ok,
								             FText::FromString(TEXT("ID must be >= 0")),
								             SInputDialogWidget::ValidationErrorTitle
							             );
						             }
						             else
						             {
							             uint32 Id;
							             LexTryParseString(Id, *InputField->GetText().ToString());
							             // ReSharper disable once CppExpressionWithoutSideEffects
							             OnConfirm.ExecuteIfBound(Id);
							             ParentWindow->RequestDestroyWindow();
						             }

						             return FReply::Handled();
					             }
				             )
			]

			+ SHorizontalBox::Slot().Padding(10)
			                        .AutoWidth()
			[
				SNew(SButton).Text(FText::FromString(TEXT("Cancel")))
				             .OnClicked_Lambda(
					             [this]() -> FReply
					             {
						             // ReSharper disable once CppExpressionWithoutSideEffects
						             OnCancel.ExecuteIfBound();
						             ParentWindow->RequestDestroyWindow();
						             return FReply::Handled();
					             }
				             )
			]
		]
	];
}

bool SInputDialogWidget::IsInputValid() const
{
	int32 Id;
	const bool bInitialValidation = InputField.IsValid() && !InputField->GetText().IsEmptyOrWhitespace();
	const bool bParseSuccess = LexTryParseString(Id, *InputField->GetText().ToString());

	if (bParseSuccess)
	{
		return Id >= 0 && bInitialValidation;
	}

	return bInitialValidation && bParseSuccess;
}
