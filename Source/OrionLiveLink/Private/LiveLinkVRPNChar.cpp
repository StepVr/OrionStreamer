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

#include "OrionLiveLinkPrivatePCH.h"
#include "LiveLinkVRPNChar.h"
#include "JsonObjectConverter.h"


#include "AllowWindowsPlatformTypes.h" 
#include <vrpn_Configure.h>  // for VRPN_CALLBACK
#include "vrpn_Tracker.h"
#include <vrpn_Text.h> 
#include "HideWindowsPlatformTypes.h"

static void ConvertTransformToFromMaya(FTransform& Transform, bool convertTranslation = true)
{
	FVector Translation = Transform.GetTranslation();
	FQuat Rotation = Transform.GetRotation();
	Rotation.X = -Rotation.X;
	Rotation.Z = -Rotation.Z;
	Transform.SetRotation(Rotation);

	if (convertTranslation)
	{
		Translation.Y = -Translation.Y;
		Transform.SetTranslation(Translation);
	}
}

void VRPN_CALLBACK
handle_tracker_pos_quat(void *userdata, const vrpn_TRACKERCB t)
{
	FLiveLinkVRPNChar *character = static_cast<FLiveLinkVRPNChar *>(userdata);

	FVector pos(t.pos[0], t.pos[1], t.pos[2]);
	FQuat quat(t.quat[0], t.quat[1], t.quat[2], t.quat[3]);
	quat.Normalize();
	if (character && character->Initialized)
	{
		FScopeLock(&character->characterSection);
		auto bone = character->Bones.FindByPredicate([t](FLiveLinkVRPNBones& bone) {return bone.ID == t.sensor; });
		if (bone)
		{
			FTransform trans(quat, pos);
			ConvertTransformToFromMaya(trans);
			bone->LocalPose = trans;
		}
	}
}

void VRPN_CALLBACK handle_text(void *userdata, const vrpn_TEXTCB t)
{
	FLiveLinkVRPNChar *character = static_cast<FLiveLinkVRPNChar *>(userdata);

	char SegId = t.message[0];
	char Total = t.message[1];
	
	if (FString(t.message) == "request_metadata")
		return;

	//if (SegId < Total)
	{
		FScopeLock(&character->characterSection);
		character->CreateHierarchy(SegId, Total, &t.message[2]);
	}
}

void FLiveLinkVRPNChar::RegisterCallbacks()
{
	tracker->register_change_handler(this, handle_tracker_pos_quat);
	text->register_message_handler(this, handle_text);
}

void FLiveLinkVRPNChar::CreateHierarchy(char SegId, char Total, const char* message)
{
	if ((SegId)-1 != (LastSegment + 1))
	{
		if (textSender)
		{
			textSender->send_message("request_metadata");

		}
	}
	LastSegment = SegId - 1;
	if (TotalSegments == -1)
	{
		TotalSegments = Total;
	}
	JsonString = JsonString + FString(message);

	if (SegId == TotalSegments)
	{
		FLiveLinkVRPNChar temp(Name);
		FJsonObjectConverter::JsonObjectStringToUStruct<FLiveLinkVRPNChar>(JsonString, &temp, 0, 0);
		Bones = temp.Bones;
		Initialized = true;

		JsonString = "";
		if (Bones.Num() == 0)
		{
			Initialized = false;
		}
	}

}

FLiveLinkVRPNChar::FLiveLinkVRPNChar()
	:Name(NAME_None), LastSegment(-1), TotalSegments(-1), Initialized(false)
{

}

FLiveLinkVRPNChar::FLiveLinkVRPNChar(const FName& name): Name(name)
{
}

FLiveLinkVRPNChar::FLiveLinkVRPNChar(const FLiveLinkVRPNChar& other)
{
	Name = other.Name;
	TStringConversion<TStringConvert <TCHAR, ANSICHAR>> a(*(other.Name.ToString()));

	tracker = (new vrpn_Tracker_Remote(a.Get(), other.tracker->connectionPtr()));
	text = (new vrpn_Text_Receiver(a.Get(), other.text->connectionPtr()));
	textSender = (new vrpn_Text_Sender(a.Get(), other.textSender->connectionPtr()));
	Bones = other.Bones;
	tracker->register_change_handler(this, handle_tracker_pos_quat);
	text->register_message_handler(this, handle_text);

}

FLiveLinkVRPNChar FLiveLinkVRPNChar::operator=(const FLiveLinkVRPNChar& other)
{
	FLiveLinkVRPNChar newChar(other);
	return newChar;
}

FLiveLinkVRPNChar::~FLiveLinkVRPNChar()
{
	if (tracker)
	{
		tracker->unregister_change_handler(this, handle_tracker_pos_quat);
		tracker->connectionPtr()->removeReference();
	}
	if (text)
	{
		text->unregister_message_handler(this, handle_text);
		text->connectionPtr()->removeReference();
	}
	if (textSender)
	{
		textSender->connectionPtr()->removeReference();
	}
}

void FLiveLinkVRPNChar::DoFK()
{
	for (auto& bone : Bones)
	{
		if (bone.PID != INDEX_NONE)
		{
			bone.GlobalPose = bone.LocalPose * Bones[bone.PID].GlobalPose;
		}
		else
		{
			bone.GlobalPose = bone.LocalPose;
		}
	}
}
