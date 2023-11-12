#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(InputDialogWidget, Log, All);

DECLARE_DELEGATE(FOnInputDialogCancel);
DECLARE_DELEGATE_OneParam(FOnInputDialogConfirm, const uint32);

class SInputDialogWidget : public SCompoundWidget
{
public:
  SLATE_BEGIN_ARGS(SInputDialogWidget)
    {
    }

    SLATE_ARGUMENT(TSharedPtr<SWindow>, ParentWindow)
    SLATE_ARGUMENT(FText, DialogTitle)
    SLATE_EVENT(FOnInputDialogCancel, OnCancel)
    SLATE_EVENT(FOnInputDialogConfirm, OnConfirm)
  SLATE_END_ARGS()

  inline static const FText ValidationErrorTitle = FText::FromString(TEXT("Error"));

  void Construct(const FArguments &InArgs);

private:
  FText DialogTitle;
  FOnInputDialogCancel OnCancel;
  FOnInputDialogConfirm OnConfirm;
  TSharedPtr<SWindow> ParentWindow;
  TSharedPtr<SEditableTextBox> InputField;

  bool IsInputValid() const;
};
