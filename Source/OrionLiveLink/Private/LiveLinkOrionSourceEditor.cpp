// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "OrionLiveLinkPrivatePCH.h"
#include "LiveLinkOrionSourceEditor.h"
#include "SBox.h"

#include "MessageEndpointBuilder.h"

#define LOCTEXT_NAMESPACE "LiveLinkOrionSourceEditor"


void SMocapSourcePicker::Construct(const FArguments& InArgs)
{
	bIsYUp = false;
//	bIsRigidBody = false;


	//ImportScale = 1.f;
	OnSourceScaleChanged = InArgs._OnSourceScaleChanged;

	ChildSlot
		[
			SNew(SBox)
			.HeightOverride(100)
			.WidthOverride(200)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.FillHeight(1.0f)
				.Padding(2.0f)
				
				[

					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.FillWidth(2.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ServerAddressLabel", "Server Address"))
							.Font(FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font")))
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(2.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.VAlign(VAlign_Center)
						[
							SAssignNew(ServerAddressBox, SEditableTextBox)
							.ToolTipText(LOCTEXT("ServerAddressTextToolTip", "The IP address for the Mocap server streaming machine"))
							.OnTextCommitted(this, &SMocapSourcePicker::OnServerAddressCommitted)
							.Font(FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font")))
						]
					]
				]
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.FillHeight(1.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.FillWidth(2.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("ServerPortLabel", "Server Port"))
							.Font(FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font")))
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(2.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.VAlign(VAlign_Center)
						[
							SAssignNew(ServerPortBox, SEditableTextBox)
							.ToolTipText(LOCTEXT("ServerAddressTextToolTip", "The port number for the Mocap streaming server"))
							.OnTextCommitted(this, &SMocapSourcePicker::OnServerPortCommitted)
							.Font(FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font")))
						]
					]
				]
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Center)
				.FillHeight(1.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.FillWidth(2.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("YUpLabel", "Streaming Y-up"))
							.Font(FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font")))
						]
					]
					+ SHorizontalBox::Slot()
					.FillWidth(2.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.VAlign(VAlign_Center)
						[
							SNew(SCheckBox)
							.ToolTipText(LOCTEXT("YUpLabelTextToolTip", "Enable if your Mocap streaming server uses Y-up"))
							.OnCheckStateChanged(this, &SMocapSourcePicker::SetYup)
						]
					]
				]
			]
		];
}



void SMocapSourcePicker::SetYup(ECheckBoxState InState)
{
	switch (InState)
	{
	case ECheckBoxState::Unchecked:
		bIsYUp = false;
		break;
	case ECheckBoxState::Checked:
		bIsYUp = true;
		break;
	default:
		bIsYUp = false;
	}
}


void SMocapSourcePicker::OnServerAddressCommitted(const FText& InputText, ETextCommit::Type CommitType)
{
	ServerIP = InputText.ToString();
}

void SMocapSourcePicker::OnServerPortCommitted(const FText& InputText, ETextCommit::Type CommitType)
{
	TDefaultNumericTypeInterface<int> Conversion;
	auto optional = Conversion.FromString(InputText.ToString(), 0);
	PortNumber = optional.Get(801);
}



const FString & SMocapSourcePicker::GetServerIP() const
{
	return ServerIP;
}




const int SMocapSourcePicker::GetPortNumber() const
{
	return PortNumber;
}




void SMocapSourcePicker::SetServerIP(const FString& _ServerIP)
{
	ServerAddressBox->SetText(FText::FromString(_ServerIP));
	ServerIP = _ServerIP;
}



void SMocapSourcePicker::SetPortNumber(const int portNumber)
{
	TDefaultNumericTypeInterface<int> Conversion;
	auto optional = Conversion.ToString(portNumber);
	ServerPortBox->SetText(FText::FromString(optional));
	PortNumber = portNumber;
}


#undef LOCTEXT_NAMESPACE