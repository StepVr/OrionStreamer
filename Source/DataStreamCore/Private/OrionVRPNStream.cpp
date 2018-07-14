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

#include "OrionVRPNStream.h"
#include "OrionDataStreamCorePrivatePCH.h"

#include "OrionVRPNChar.h"
#include "AllowWindowsPlatformTypes.h" 
#include "vrpn_Tracker.h"
#include <vrpn_Text.h> 
#include "HideWindowsPlatformTypes.h"

TMap<FString, UOrionVRPNStream*> UOrionVRPNStream::Streams;

int VRPN_CALLBACK omsg_handler(void * Streamerptr, vrpn_HANDLERPARAM p)
{

	if (!Streamerptr)
	{
		return -1;
	}
	UOrionVRPNStream* streamer = (UOrionVRPNStream*)Streamerptr;
	const char *sender_name = streamer->GetSocket()->sender_name(p.sender);
	const char *type_name = streamer->GetSocket()->message_type_name(p.type);
	if (sender_name != nullptr)
	{
		streamer->InitialiseCharacter(p.sender, FString(sender_name));
	}
	return -1; // Do not log the message
}

// Default constructor.
FOrionDataStreamToSkeletonBinding::FOrionDataStreamToSkeletonBinding()
	: mStream(nullptr), mBound(false)
{
}

FOrionDataStreamToSkeletonBinding::~FOrionDataStreamToSkeletonBinding()
{
	UE4BoneIndices.Empty();
	mBound = false;
}

// Bind the given Strean to the given skeleton and store the result.
bool FOrionDataStreamToSkeletonBinding::BindToSkeleton(UOrionVRPNStream* Stream, const FString& subject, USkeleton* skeleton, USkeletalMeshComponent* component)
{
	if (Stream == nullptr)
	{
		UE_LOG(LogLADataStreamCore, Warning, TEXT("Invalid Mocap Stream for skeleton binding"));
		return false;
	}
	mStream = Stream;
	mSubjectName = subject;
	int32 numBoneDefs = INDEX_NONE;
	if (mStream->isConnected())
	{
		//刷新一帧数据
		mStream->GetFrame();

		//获取需要修改的关节数
		mStream->GetSegmentCountForSubject(subject, numBoneDefs);
		if (numBoneDefs == INDEX_NONE || numBoneDefs == 0)
		{
			return false;
		}

		UE4BoneIndices.Empty(numBoneDefs);
		for (int32 i = 0; i < numBoneDefs; ++i)
		{
			int32 ue4BoneIndex = INDEX_NONE;
			FString Name;
			FString ParentName;

			//从Orion中获取每个节点的名字
			mStream->GetSegmentNameForSubject(subject, i, Name);
			mStream->GetSegmentParentNameForSubject(subject, Name, ParentName);

			//从引擎使用的骨骼中找到对应名字对应的序列号缓存
			ue4BoneIndex = component->GetBoneIndex(FName(*Name));
			UE4BoneIndices.Add(ue4BoneIndex);
		}
		mBound = true;
	}
	else
	{
		mBound = false;
	}
	return mBound;
}

bool FOrionDataStreamToSkeletonBinding::UpdatePose(TArray<FTransform>& outPose)
{
	if (mStream == nullptr)
	{
		UE_LOG(LogLADataStreamCore, Warning, TEXT("Stream object is not valid"));
		return false;
	}

	if (!mBound)
	{
		UE_LOG(LogLADataStreamCore, Warning, TEXT("Skeleton not bound to a valid stream"));
		return false;
	}

	if (!mStream->isConnected())
		return false;

	return  (mStream->GetPoseForSubject(this, mSubjectName, outPose));
}

float FOrionDataStreamToSkeletonBinding::GetFigureScale()
{
	if (mStream)
	{
		return mStream->GetSubjectScale(mSubjectName);
	}
	return 1.f;
}

// Access the UE4 bone index given the bone def index.
int32 FOrionDataStreamToSkeletonBinding::GetUE4BoneIndex(int32 boneDefIndex) const
{
	if (UE4BoneIndices.IsValidIndex(boneDefIndex))
		return UE4BoneIndices[boneDefIndex];
	return -1;
}

int FOrionDataStreamToSkeletonBinding::GetNumberOfBones() const
{
	return UE4BoneIndices.Num();
}

void FOrionDataStreamToSkeletonBinding::GetUE4BoneIndices(TArray<int32>& InBondsID)
{
	InBondsID.Empty(UE4BoneIndices.Num());

	InBondsID = UE4BoneIndices;
}

void FOrionDataStreamToSkeletonBinding::SetUE4BoneIndices(TArray<int32>& InBondsID)
{
	UE4BoneIndices.Empty(InBondsID.Num());

	UE4BoneIndices = InBondsID;

	mBound = true;
}

bool FOrionDataStreamToSkeletonBinding::IsBound()
{
	return mBound;
}

const FString& FOrionDataStreamToSkeletonBinding::GetSubjectName()
{
	return mSubjectName;
}

UOrionVRPNStream::UOrionVRPNStream(const FObjectInitializer& PCIP)
	: Super(PCIP), m_port(3883), KillThread(false), bIsInitialized(false), Thread(nullptr)
{
	iDataPort = 3883;
	is_realtime = true;
}

ELAResult UOrionVRPNStream::Connect(FString server_ip, int portNumber)
{
	if (IsValidLowLevel())
	{
		if (isConnected())
		{
			Disconnect();
		}
		else
		{
			if (Thread)
			{
				KillThread = true;
				Stop();
				Thread = nullptr;
			}
			characters.Empty();
			AvatarNameMap.Empty();
		}

		_server_ip = server_ip;
		m_port = portNumber;

		//重新连接
		{
			FScopeLock Lock(&SocketSection);
			m_socket.Reset();
		}
		FString client = server_ip + ":" + FString::FromInt(m_port);
		TStringConversion<TStringConvert <TCHAR, ANSICHAR>> a(*client);
		m_socket = TSharedPtr<vrpn_Connection>(vrpn_get_connection_by_name(a.Get(), "t"));

		if (m_socket.IsValid())
		{
			m_socket->register_log_filter(omsg_handler, this);
			m_socket->mainloop();
			m_socket->setAutoDeleteStatus(false);
			//TODO: start parsing datagrams thread call startRead in a thread somehow :)
			if (!Thread)
			{
				static int ThreadIndex = 0;
				FString ThreadName = FString::Printf(TEXT("Streamer_%d"), ThreadIndex);
				ThreadIndex++;
				KillThread = false;
				Thread = FRunnableThread::Create(this, *ThreadName, 0, EThreadPriority::TPri_Normal, FPlatformAffinity::GetNoAffinityMask());
			}

		}
	}
	return ELAResult::ELASuccess;
}

uint32 UOrionVRPNStream::Run()
{
	while (!KillThread && isConnected())
	{
		{
			FScopeLock lock(&section);

			if (m_socket.IsValid())
			{
				m_socket->mainloop();
				for (auto& character : characters)
				{
					if (!character.Value->Initialized)
					{
						character.Value->textSender->mainloop();
						character.Value->text->mainloop();
					}
					character.Value->tracker->mainloop();					
				}
			}
		}
		FPlatformProcess::Sleep(0.0f);
	}
	if (KillThread)
	{
		Stop();
	}
	return 0;
}


UOrionVRPNStream::~UOrionVRPNStream()
{
	Disconnect();
}

bool UOrionVRPNStream::InitialiseCharacter(int senderId, const FString& senderName)
{
	if (characters.Contains(senderId))
	{
		return true;
	}
	TSharedPtr<FOrionVRPNChar> character = TSharedPtr<FOrionVRPNChar>(new FOrionVRPNChar(*(senderName)));
	FString client = _server_ip + ":" + FString::FromInt(m_port);
	FString text = senderName + "@" + client;

	TStringConversion<TStringConvert <TCHAR, ANSICHAR>> a(*senderName);
	character->tracker = (new vrpn_Tracker_Remote(a.Get(), m_socket.Get()));
	character->tracker->connectionPtr()->setAutoDeleteStatus(false);
	character->text = (new vrpn_Text_Receiver(a.Get(), m_socket.Get()));
	character->text->connectionPtr()->setAutoDeleteStatus(true);
	TStringConversion<TStringConvert <TCHAR, ANSICHAR>> b(*text);
	character->textSender = (new vrpn_Text_Sender(a.Get(), m_socket.Get()));
	character->textSender->connectionPtr()->setAutoDeleteStatus(true);
	if (character->textSender)
	{
		character->textSender->send_message("request_metadata");

	}
	character->RegisterCallbacks();
	characters.Add(senderId, character);
	AvatarNameMap.Add(senderId, senderName);
	return true;
}

const FString& UOrionVRPNStream::GetServerIP()
{
	return _server_ip;
}

ELAResult UOrionVRPNStream::Disconnect()
{
	if (m_socket.IsValid())
	{
		{
			//Release reference to the XsSocket to close it.		
			FScopeLock lock(&section);
			m_socket->removeReference();
			m_socket.Reset();
			FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*FString("t"));
		}
		KillThread = true;
		if (Thread)
		{
			Thread->Kill(true);
			delete Thread;
			Thread = nullptr;
		}
		AvatarNameMap.Empty();
		characters.Empty();
	}
	return ELAResult::ELASuccess;
}

ELAResult UOrionVRPNStream::GetSegmentCountForSubject(const FString& subjectName, int& count)
{
	unsigned int ind = GetSubjectIndex(subjectName);
	count = -1;
	if (ind == -1)
	{
		UE_LOG(LogLADataStreamCore, Warning, TEXT("Invalid Subject name"));
		return ELAResult::ELAError;
	}

	count = characters[ind]->Bones.Num();
	return ELAResult::ELASuccess;
}

ELAResult UOrionVRPNStream::GetSegmentNameForSubject(const FString& subjectName, int index, FString& segName)
{
	int ind = GetSubjectIndex(subjectName);
	if (ind == -1)
	{
		UE_LOG(LogLADataStreamCore, Warning, TEXT("Invalid Subject name"));
		return ELAResult::ELAError;
	}
	segName = characters[ind]->Bones[index].Name.ToString();
	segName.ReplaceInline(ANSI_TO_TCHAR("-"), ANSI_TO_TCHAR("_"));
	return ELAResult::ELASuccess;

}

ELAResult UOrionVRPNStream::GetSegmentParentNameForSubject(const FString& subjectName, FString segName, FString& parentSegName)
{
	int SubIndex, SegIndex, parentID;
	SubIndex = GetSubjectIndex(subjectName);
	if (SubIndex == -1)
	{
		UE_LOG(LogLADataStreamCore, Warning, TEXT("Invalid Subject name"));
		return ELAResult::ELAError;
	}

	SegIndex = GetSegmentIndex(SubIndex, segName);
	if (SegIndex == -1 || (!characters.Contains(SubIndex)))
	{
		parentSegName = "";
		UE_LOG(LogLADataStreamCore, Warning, TEXT("Invalid Subject name"));
		return ELAResult::ELAError;
	}
	parentID = characters[SubIndex]->Bones[SegIndex].PID;
	if (characters[SubIndex]->Bones.IsValidIndex(parentID))
	{
		parentSegName = characters[SubIndex]->Bones[parentID].Name.ToString();
		return ELAResult::ELASuccess;

	}
	return ELAResult::ELAError;
}

int UOrionVRPNStream::GetSegmentIndex(int SubjectIndex, FString SegmentName)
{
	if (SubjectIndex != INDEX_NONE && characters.Contains(SubjectIndex))
	{
		int index = characters[SubjectIndex]->Bones.IndexOfByPredicate([&SegmentName](const FOrionVRPNBones& bone) {return SegmentName == bone.Name.ToString(); });
		return index;
	}
	return INDEX_NONE;
}

int UOrionVRPNStream::GetSubjectIndex(const FString& SubjectName)
{
	if (SubjectName.IsEmpty() || (!AvatarNameMap.FindKey(SubjectName)))
		return -1;
	int id = *AvatarNameMap.FindKey(SubjectName);
	return id;
}

float UOrionVRPNStream::GetSubjectScale(const FString& SubjectName)
{
	int SubIndex = GetSubjectIndex(SubjectName);
	if (characters[SubIndex]->Bones.Num() <= 0)
		return false;
	return characters[SubIndex]->Bones[0].FigScale;
}

bool UOrionVRPNStream::GetPoseForSubject(FOrionDataStreamToSkeletonBinding* skeletonBinding, FString& subjectName, TArray<FTransform>& outPose)
{
	if (outPose.Num() == 0)
	{
		return false;
	}
	{
		if (GetFrame() != ELAResult::ELASuccess)
			return false;
	}
	int segCount;
	GetSegmentCountForSubject(subjectName, segCount);
	int SubIndex = GetSubjectIndex(subjectName);
	if (SubIndex == INDEX_NONE)
	{
		return false;
	}
	if (characters[SubIndex]->Bones.Num() < 0)
		return false;
	{
		FScopeLock(&characters[SubIndex]->characterSection);
		const auto localSeg = characters[SubIndex]->Bones;
		for (int j = 0; j < segCount; ++j)
		{
			outPose[j] = localSeg[j].LocalPose;
		}
	}
	return true;
}

UOrionVRPNStream* UOrionVRPNStream::Get(const FString& serverIP)
{
	auto stream = Streams.Find(serverIP);
	if (stream)
	{
		if ((*stream)->IsValidLowLevel())
		{
			(*stream)->RefCount++;
			return *stream;
		}
		Streams.Remove(serverIP);
	}
	auto newStream = NewObject<UOrionVRPNStream>();
	newStream->AddToRoot();
	newStream->RefCount = 1;
	Streams.Add(serverIP);
	Streams[serverIP] = newStream;

	return newStream;
}

void UOrionVRPNStream::Remove(const FString serverIP)
{
	auto stream = Streams.Find(serverIP);
	if (stream)
	{
		if ((*stream)->IsValidLowLevel())
		{
			(*stream)->RefCount--;

			if ((*stream)->RefCount <= 0)
			{
				(*stream)->Disconnect();
				Streams.Remove(serverIP);
			}
		}
	}
}

void UOrionVRPNStream::Destory()
{
	if (this->IsValidLowLevel())
	{
		RefCount--;
		if (RefCount <= 0)
		{
			Disconnect();
			if (Thread)
			{
				delete Thread;
				Thread = nullptr;
			}
			ConditionalBeginDestroy(); //instantly clears UObject out of memory
			RemoveFromRoot();
		}
	}
}

TSharedPtr<vrpn_Connection>& UOrionVRPNStream::GetSocket()
{
	return m_socket;
}

void UOrionVRPNStream::BeginDestroy()
{
	RefCount--;
	if (RefCount <= 0)
	{
		Streams.Remove(_server_ip);
		Disconnect();
		Super::BeginDestroy();
	}
	Super::BeginDestroy();
}

void UOrionVRPNStream::FinishDestroy()
{
	if (RefCount <= 0)
	{
		Super::FinishDestroy();
	}
	Super::FinishDestroy();
}

bool UOrionVRPNStream::isConnected()
{
	FScopeLock lock(&SocketSection);

	if (!m_socket.IsValid())
	{
		return false;
	}

	if (KillThread)
	{
		return false;
	}

	//
	return m_socket->connected() && m_socket->doing_okay();
}

ELAResult UOrionVRPNStream::GetFrame()
{
	if (is_realtime && !bIsInitialized)
	{
		bIsInitialized = true;
		for (auto character : characters)
		{
			bIsInitialized &= character.Value->Initialized;
		}
	}
	{
		FScopeLock lock(&section);
		for (auto character : characters)
		{
			character.Value->DoFK();
		}
	}
	return ELAResult::ELASuccess;
}
