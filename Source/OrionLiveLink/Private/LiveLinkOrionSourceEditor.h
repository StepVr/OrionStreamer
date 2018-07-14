// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "SListView.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Guid.h"


class SEditableTextBox;


DECLARE_DELEGATE_OneParam(FOnSourceScaleChanged, float /** SourceScale */);

class SMocapSourcePicker : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SMocapSourcePicker) {}
	SLATE_EVENT(FOnSourceScaleChanged, OnSourceScaleChanged)
		SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs);

	const FString& GetServerIP() const;
	const FString& GetSubjectName() const;
	


	const int GetPortNumber() const;

	void SetServerIP(const FString& ServerIP);

	void SetPortNumber(const int portNumber);


	bool IsYup() { return bIsYUp; };
private:

	/** Delegate called when changing Mocap source providers */

	void OnServerAddressCommitted(const FText& InputText, ETextCommit::Type CommitType);
	void OnServerPortCommitted(const FText& InputText, ETextCommit::Type CommitType);
	void SetYup(ECheckBoxState InState);

	/** Get the content for the drop-down menu for picking providers */
	TSharedRef<SWidget> OnGetMenuContent() const;

	/** Get the button text for the drop-down */
	FText OnGetButtonText() const;

	/** Get the text to be displayed given the name of the provider */
	FText GetProviderText(const FName& InName) const;

	TSharedRef<SWidget> OnGetTemplateMenuContent() const;

	FText OnGetTemplateButtonText() const;
	FString ServerIP;
	int PortNumber;
	bool bIsYUp;
	TSharedPtr<SEditableTextBox> ServerAddressBox;
	TSharedPtr<SEditableTextBox> ServerPortBox;
	FOnSourceScaleChanged OnSourceScaleChanged;
};