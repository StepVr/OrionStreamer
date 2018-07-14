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

#include "AnimNode_OrionDataStream.h"
#include "OrionDataStreamCorePrivatePCH.h"
#include "StepVrServerModule.h"
#include "StepVrGlobal.h"
#include "AnimInstanceProxy.h"


#define STEPVR_SERVER_ISRUN StepVrGlobal::ServerIsRun()

FAnimNode_OrionDataStream::FAnimNode_OrionDataStream()
	: PortNumber(3883), Reconnect(false), Streamer(nullptr), numberOfAttempts(0), ReallyReconnect(false), previousPort(3883)
{
	ServerName = NAME_None;
	SubjectName = NAME_None;
}

FAnimNode_OrionDataStream::~FAnimNode_OrionDataStream()
{
	if (Streamer)
	{
		Streamer->Remove(StrServerName);
	}


	//FArrayWriter Writer;
	//Writer << SaveArr;

	//FString TArgetPath = FPaths::ProjectSavedDir() + TEXT("cache.txt");
	//UE_LOG(LogTemp,Warning,TEXT("----%d"), Writer.Num());
	//FFileHelper::SaveArrayToFile(Writer, *TArgetPath);
}


void FAnimNode_OrionDataStream::BindSkeleton(FAnimInstanceProxy* AnimInstanceProxy)
{

	USkeleton* skeleton = AnimInstanceProxy->GetSkeleton();
	if (skeleton != nullptr && StrSubjectName != "None")
	{
		//绑定的缓存
		mSkeletonBinding = FOrionDataStreamToSkeletonBinding();
		ReallyReconnect = mSkeletonBinding.BindToSkeleton(Streamer, StrSubjectName, skeleton, AnimInstanceProxy->GetSkelMeshComponent());
		ReallyReconnect = ReallyReconnect && Reconnect;
	}
}

void FAnimNode_OrionDataStream::IntializeStreamer(FAnimInstanceProxy* AnimInstanceProxy)
{
	//初始化本地的数据流，绑定骨架，缓存需要修改的骨架ID（不是所有骨架都需要修改）
	//模拟角色需要走网络数据，不需要链接本地Streamer
	//模拟角色需要绑定骨架及缓存骨架ID，之后在update的时候用网络数据修改对应的骨架

	if (IsLocalControlled)
	{
		if (StrServerName != "None" && (!Streamer || Streamer->GetServerIP() != StrServerName))
		{
			if (Streamer)
			{
				Streamer->Remove(Streamer->GetServerIP());
				Streamer = nullptr;
			}
			FString ConnectionID = StrServerName + FString::Printf(TEXT("%d"), PortNumber);
			Streamer = UOrionVRPNStream::Get(ConnectionID);
		}
		if (Streamer)
		{
			if (Streamer->Connect(StrServerName, PortNumber) == ELASuccess)
			{
				numberOfAttempts = 0;

				//绑定骨架
				BindSkeleton(AnimInstanceProxy);
				ReallyReconnect = false;
			}
		}
	}
}

void FAnimNode_OrionDataStream::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	//蓝图开始时进行一次初始化
	FAnimNode_Base::Initialize_AnyThread(Context);
	InPose.Initialize(Context);
	
	// Forward to the incoming pose link.
	check(Context.AnimInstanceProxy != nullptr);

	StrServerName = ServerName.ToString();
	StrSubjectName = SubjectName.ToString();
	auto AnimInstanceProxy = Context.AnimInstanceProxy;

	//初始化数据流
	IntializeStreamer(AnimInstanceProxy);
	previousPort = PortNumber;

}

void FAnimNode_OrionDataStream::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("LADataStream_Update"), STAT_StatsOrionEval, STATGROUP_Orion);
	InPose.Update(Context);
	EvaluateGraphExposedInputs.Execute(Context);

	//是否是本地
	if (IsLocalControlled)
	{
		//重复确认是否已经连接到本地的数据流
		auto temp = ServerName.ToString();
		if (temp != StrServerName || Streamer == nullptr)
		{
			StrServerName = temp;
			IntializeStreamer(Context.AnimInstanceProxy);
		}


		StrSubjectName = SubjectName.ToString();


		//重复确认是否已经绑定骨架
		if (!mSkeletonBinding.IsBound() || ((SubjectName.IsNone()) && mSkeletonBinding.GetSubjectName() != StrSubjectName))
		{
			BindSkeleton(Context.AnimInstanceProxy);
		}

		//更新本地数据
		if (Streamer)
		{
			Streamer->GetFrame();
		}
	}
	else if(STEPVR_SERVER_ISRUN)
	{
		//刷新模拟数据
		FrameCacheData.SkeletionIDs.Empty();
		FrameCacheData.SkeletonInfos.Empty();
		STEPVR_SERVER->IkinemaGetData(OwnerPlayerID, FrameCacheData);

		//绑定数据
		if (!mSkeletonBinding.IsBound() && FrameCacheData.SkeletionIDs.Num() > 0)
		{
			mSkeletonBinding.SetUE4BoneIndices(FrameCacheData.SkeletionIDs);
		}
	}
}

FGraphEventRef oExecOnGameThread(TFunction<void()> funcLambda)
{
	FGraphEventRef funcTask = FFunctionGraphTask::CreateAndDispatchWhenReady(funcLambda, TStatId(), NULL, ENamedThreads::GameThread);
	return funcTask;
}

void FAnimNode_OrionDataStream::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("LADataStream_Eval"), STAT_StatsOrionEval, STATGROUP_Orion);
	check(Output.AnimInstanceProxy->GetSkeleton() != nullptr);
	Output.Pose.ResetToRefPose();

	if (!mSkeletonBinding.IsBound() || !Reconnect)
	{
		return;
	}

	//骨骼姿态
	TArray<FTransform> Bones;

	//需要控制骨架数
	int numberOfBones = 0;

	//本地从Orion获取
	if (IsLocalControlled)
	{
		//确认是否链接，模拟端不需处理
		if (!Streamer || !Streamer->isConnected())
		{
			UE_LOG(LogLADataStreamCore, Warning, TEXT("LADataStream::Evaluate() Not connected to Mocap server."));
			return;
		}

		numberOfBones = mSkeletonBinding.GetNumberOfBones();
		if (numberOfBones == 0)
		{
			UE_LOG(LogLADataStreamCore, Warning, TEXT("LADataStream::Evaluate() Couldn't get a pose from Mocap source."));
			return;
		}
	}
	else if(STEPVR_SERVER_ISRUN)
	{
		//从Server中获取
		numberOfBones = FrameCacheData.SkeletionIDs.Num();
	}

	//从Streamer获取骨架的姿态，同步时直接替换为网络数据
	Bones.Init(FTransform::Identity, numberOfBones);


	if (IsLocalControlled)
	{
		//获取本地姿态
		if (!mSkeletonBinding.UpdatePose(Bones) || Bones.Num() == 0)
		{
			return;
		}

		//同步
		if (STEPVR_SERVER_ISRUN)
		{
			FrameSendData.PlayerID = OwnerPlayerID;
			mSkeletonBinding.GetUE4BoneIndices(FrameSendData.SkeletionIDs);
			FrameSendData.SkeletonInfos = Bones;

			//SaveArr.Add(Writer);

			STEPVR_SERVER->IkinemaSendData(FrameSendData);
		}
	}
	else if(STEPVR_SERVER_ISRUN)
	{
		Bones = FrameCacheData.SkeletonInfos;
	}

	//没有需要修改的直接退出
	if (Bones.Num() == 0)
	{
		return;
	}

	//不用修改直接使用网络数据或者本地数据进行修改骨骼
	for (int32 i = 0; i < Bones.Num(); i++)
	{
		//获取骨架节点号
		auto ue4Index = mSkeletonBinding.GetUE4BoneIndex(i);
		if (ue4Index != INDEX_NONE)
		{
			auto bone = Bones[i];
			if (i == 0 && ue4Index == i)
			{
				const FQuat rot(FVector(1, 0, 0), FMath::DegreesToRadians(-90));
				bone = bone * rot;
			}
			
			if(StreamYUp && i == 0)
			{
				const FQuat rot(FVector(1, 0, 0), FMath::DegreesToRadians(-90));
				bone = bone * rot;
			}
			auto SkeletonIndex = Output.Pose.GetBoneContainer().MakeCompactPoseIndex(FMeshPoseBoneIndex(ue4Index));
			
			//确定某骨架姿态
			Output.Pose[SkeletonIndex] = bone;
		}
	}


	float scale = 1;
	if (IsLocalControlled)
	{
		mSkeletonBinding.GetFigureScale();
	}
	
	auto SkelMesh = Output.AnimInstanceProxy->GetSkelMeshComponent(); 

	if (!Output.IsNormalized())
	{
		// Show bone transform with some useful debug info
		for (const FTransform& Bone : Output.Pose.GetBones())
		{
			if (!Bone.IsRotationNormalized())
			{
				ensureMsgf(Bone.IsRotationNormalized(), TEXT("Bone Rotation not normalized for bone"));
			}
		}
	}
	if (ApplyOrionScale && SkelMesh->GetComponentScale().X != scale)
	{
		oExecOnGameThread(
			[scale, SkelMesh]() {

			SkelMesh->SetRelativeScale3D(FVector(scale ? scale : 1.f));

		});
	}

}

void FAnimNode_OrionDataStream::CacheBones_AnyThread(const FAnimationCacheBonesContext & Context)
{
	InPose.CacheBones(Context);
}
