#include "InputDialogWidget.h"

BEGIN_DEFINE_SPEC(
  FInputDialogWidgetSpec,
  "BlockadeLabs.InputDialogWidget.Unit",
  EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask
)
  TSharedPtr<SInputDialogWidget> InputDialogWidget;
END_DEFINE_SPEC(FInputDialogWidgetSpec)

void FInputDialogWidgetSpec::Define()
{
  LatentBeforeEach(
    [this](const FDoneDelegate& Done)
    {
      AsyncTask(
        ENamedThreads::GameThread,
        [this, Done]()
        {
          InputDialogWidget = SNew(SInputDialogWidget);
          TestTrue("InputDialogWidget is valid", InputDialogWidget.IsValid());
          Done.Execute();
        }
      );
    }
  );

  Describe(
    "InputDialogWidget",
    [this]()
    {
      Describe(
        "IsInputValid",
        [this]()
        {
          LatentBeforeEach(
            [this](const FDoneDelegate& Done)
            {
              AsyncTask(
                ENamedThreads::GameThread,
                [this, Done]()
                {
                  InputDialogWidget->GetInputField()->SetText(FText::FromString(TEXT("")));
                  Done.Execute();
                }
              );
            }
          );

          LatentIt(
            "Should return false when input empty",
            EAsyncExecution::ThreadPool,
            [this](const FDoneDelegate& Done)
            {
              AsyncTask(
                ENamedThreads::GameThread,
                [this, Done]()
                {
                  TestFalse("Input is not valid", InputDialogWidget->IsInputValid());
                  Done.Execute();
                }
              );
            }
          );

          LatentIt(
            "Should return false when input is NaN",
            EAsyncExecution::ThreadPool,
            [this](const FDoneDelegate& Done)
            {
              AsyncTask(
                ENamedThreads::GameThread,
                [this, Done]()
                {
                  InputDialogWidget->GetInputField()->SetText(FText::FromString(TEXT("Im Not A Number")));
                  TestFalse("Input is not valid", InputDialogWidget->IsInputValid());
                  Done.Execute();
                }
              );
            }
          );

          LatentIt(
            "Should return false when input is negative",
            EAsyncExecution::ThreadPool,
            [this](const FDoneDelegate& Done)
            {
              AsyncTask(
                ENamedThreads::GameThread,
                [this, Done]()
                {
                  InputDialogWidget->GetInputField()->SetText(FText::FromString(TEXT("-1234")));
                  TestFalse("Input is not valid", InputDialogWidget->IsInputValid());
                  Done.Execute();
                }
              );
            }
          );

          LatentIt(
            "Should return true when valid non negative number entered",
            EAsyncExecution::ThreadPool,
            [this](const FDoneDelegate& Done)
            {
              AsyncTask(
                ENamedThreads::GameThread,
                [this, Done]()
                {
                  InputDialogWidget->GetInputField()->SetText(FText::FromString(TEXT("0")));
                  TestTrue("Input is valid", InputDialogWidget->IsInputValid());

                  InputDialogWidget->GetInputField()->SetText(FText::FromString(TEXT("1234")));
                  TestTrue("Input is valid", InputDialogWidget->IsInputValid());

                  Done.Execute();
                }
              );
            }
          );
        }
      );
    }
  );
}
