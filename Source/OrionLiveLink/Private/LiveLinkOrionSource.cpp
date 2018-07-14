// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "OrionLiveLinkPrivatePCH.h"
#include "LiveLinkOrionSource.h"
#include "LiveLinkMessages.h"
#include "ILiveLinkClient.h"
#include "LiveLinkVRPNChar.h"
#include "MessageEndpointBuilder.h"

void FLiveLinkOrionSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid)
{
	Client = InClient;
	SourceGuid = InSourceGuid;


	
	DataStream = ULiveLinkVRPNStream::Get(SourceMachineName.ToString());
	if (DataStream && DataStream->Connect(SourceMachineName.ToString(), PortNumber) == ELASuccess)
	{
		
		//connect twice as sometimes first fails to get character
		if (DataStream->Connect(SourceMachineName.ToString(), PortNumber) == ELASuccess)
		{																												 
			DataStream->GetFrame();																						 
			for (auto subject : DataStream->AvatarNameMap)																 
			{																											 
				int32 numBoneDefs = 0;																					 
				while (!numBoneDefs)																					 
																														 
				{																										 
					FPlatformProcess::Sleep(0.5);																		 
					DataStream->GetSegmentCountForSubject(subject.Value, numBoneDefs);									 
				}																										 
																														 
				TArray<FName> BoneNames;																				 
				TArray<int32> ParentBones;																				 
				for (auto Bone : DataStream->characters[subject.Key]->Bones)											 
				{																										 
					int32 ue4BoneIndex = INDEX_NONE;																	 
					BoneNames.Add(Bone.Name);																			 
					ParentBones.Add(Bone.PID);																			 
				}																										 
																														 
																														 
				FLiveLinkRefSkeleton Skel;																				 
				Skel.SetBoneNames(BoneNames);																			 
				Skel.SetBoneParents(ParentBones);																		 
																														 
																														 
				Client->PushSubjectSkeleton(SourceGuid,DataStream->characters[subject.Key]->Name, Skel);
			}																											 
		}

	}

	DataStream->SetCharacterCallBack(this, &FLiveLinkOrionSource::HandleSubjectFrame);
	


}



bool FLiveLinkOrionSource::IsSourceStillValid()
{
	return true;
}


bool FLiveLinkOrionSource::RequestSourceShutdown()
{


	DataStream->RemoveCharacterCallBack();
	return true;
}




void FLiveLinkOrionSource::HandleSubjectFrame(const TSharedPtr<FLiveLinkVRPNChar> character)
{
	
			TArray<FTransform> Bones;

			for (int i = 0; i < character->Bones.Num(); i++)
			{
				if (i == 0 && Yup)
				{
					Bones.Add(character->Bones[i].LocalPose *FQuat(FVector(1, 0, 0), FMath::DegreesToRadians(-90.f)));
				}
				else
				{
					Bones.Add(character->Bones[i].LocalPose);
				}
			}
			TArray<FLiveLinkCurveElement> Curves;
			FLiveLinkFrameData data;
			data.CurveElements = Curves;
			data.Transforms = Bones;
			Client->PushSubjectData(SourceGuid, character->Name, data);
		
}