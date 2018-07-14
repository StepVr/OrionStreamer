/* Copyright (C) 2006-2017, IKinema Ltd. All rights reserved.
*
* IKinema API Library; SDK distribution
*
* This file is part of IKinema LiveAction project http://www.ikinema.com
*
* Your use and or redistribution of this software in source and / or binary form, with or without
* modification, is subject to:
* (i) your ongoing acceptance of and compliance with the terms and conditions of
* the IKinema License Agreement; and
*
* (ii) your inclusion of this notice in any version of this software that you use
* or redistribute.
*
*
* A copy of the IKinema License Agreement is available by contacting
* IKinema Ltd., http://www.ikinema.com, support@ikinema.com
*
*/

#pragma once
#include "OrionVRPNChar.generated.h"
class vrpn_Text_Sender;
class vrpn_Text_Receiver;
class vrpn_Tracker_Remote;

USTRUCT()
struct FOrionVRPNBones
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FName Name;

	UPROPERTY()
	FTransform Rest;

	UPROPERTY()
	FTransform LocalPose;

	UPROPERTY()
	FTransform GlobalPose;

	UPROPERTY()
	int32 PID;

	UPROPERTY()
	int32 ID;

	UPROPERTY()
	float FigScale;
};

USTRUCT()
struct FOrionVRPNChar 
{
	GENERATED_USTRUCT_BODY()

	FOrionVRPNChar();
	FOrionVRPNChar(const FName& name);;
	FOrionVRPNChar(const FOrionVRPNChar& other);
	FOrionVRPNChar operator=(const FOrionVRPNChar& other);
	~FOrionVRPNChar();

	FCriticalSection characterSection;

	void CreateHierarchy(char SegId, char Total, const char* message);
	void RegisterCallbacks();

	UPROPERTY()
	TArray<FOrionVRPNBones> Bones;

	int TotalSegments = -1;
	int LastSegment = -1;
	bool Initialized = false;
	FString JsonString = "";
	FName Name;
	void DoFK();

	vrpn_Tracker_Remote* tracker = nullptr;
	vrpn_Text_Receiver*  text = nullptr;
	vrpn_Text_Sender* textSender = nullptr;
};
