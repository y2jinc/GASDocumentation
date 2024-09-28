// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCheatManager.h"

#include "BlueprintActionDatabase.h"
#include "Containers/StringConv.h"


void FMusicContext::Play()
{
	if (IsPlaying())
	{
		return;
	}

	bPlayed = true;
	bPaused = false;
	bStoped = false;
	
	UE_LOG(LogTemp, Warning, TEXT("\t\t%s - %s"), ANSI_TO_TCHAR(__FUNCTION__), *Key.ToString());
	
}

void FMusicContext::Resume()
{
	if (!IsPaused())
    {
    	return;
    }
    
    bPlayed = true;
    bPaused = false;
    bStoped = false;
    	
    UE_LOG(LogTemp, Warning, TEXT("\t\t%s - %s"), ANSI_TO_TCHAR(__FUNCTION__), *Key.ToString());
}

void FMusicContext::Stop(bool bInPause)
{
	bStoped = true;
	bPaused = bInPause;
	bPlayed = false;

	UE_LOG(LogTemp, Warning, TEXT("\t\t%s - %s Pause : %d"), ANSI_TO_TCHAR(__FUNCTION__), *Key.ToString(), static_cast<int32>(bInPause));
}

bool FMusicContext::IsPlaying() const
{
	return bPlayed;
}

bool FMusicContext::IsStopped() const
{
	return bStoped;
}

bool FMusicContext::IsPaused() const
{
	return bPaused;
}

void FMusicContext::DisplayInfo()
{
	UE_LOG(LogTemp, Warning, TEXT("\t\tFMusicContext Name : %s, Played : %d"),
    			*Key.ToString(), static_cast<int32>(bPlayed));
}

bool FMusicContextContainer::IsActivate() const
{
	for (const auto& TmpMusicContext : MusicContexts)
	{
    	if (TmpMusicContext->IsPlaying())
    	{
    		return true;
    	}
    }
		
	return false;
}

bool FMusicContextContainer::GetPauseOrStopOnPlay() const
{
	return false;
}

int32 FMusicContextContainer::GetPriority() const
{
	return Priority;
}

const FMusicContext* FMusicContextContainer::GetMusicContext(const FName& InKey) const
{
	const auto* FindMusicConext = MusicContexts.FindByPredicate([InKey](const TSharedPtr<FMusicContext>& TmpContext) -> bool
	{
		return InKey == TmpContext->Key;
	});

	return (FindMusicConext && FindMusicConext->IsValid())? FindMusicConext->Get() : nullptr;
}

void FMusicContextContainer::Play()
{
	bStopped = false;
	bPaused = false;
	
	for (const auto& TmpMusicContext : MusicContexts)
	{
		if (TmpMusicContext->IsPaused())
		{
			TmpMusicContext->Play();
		}
		TmpMusicContext->Play();
	}
}

void FMusicContextContainer::Stop(const FName& InKey, bool bInPause)
{
	const auto* FindMusicConext = MusicContexts.FindByPredicate([InKey](const TSharedPtr<FMusicContext>& TmpContext) -> bool
	{
		return InKey == TmpContext->Key;
	});

	if (FindMusicConext && FindMusicConext->IsValid())
	{
		(*FindMusicConext)->Stop(bInPause);
	}
}

void FMusicContextContainer::StopAll(bool bInPause)
{
	UE_LOG(LogTemp, Warning, TEXT("\tMusicContextContainer::StopAll Priority : %d, Puase : %d"),
    			GetPriority(), static_cast<int32>(bInPause));
	
	bStopped = true;
	bPaused = bInPause;
	for (const auto& TmpMusicContext : MusicContexts)
	{
    	TmpMusicContext->Stop(bInPause);
    }
}

void FMusicContextContainer::AddContext(FName InKey, int32 InPriority)
{
	TSharedPtr<FMusicContext>* FindContext = MusicContexts.FindByPredicate([InKey](const TSharedPtr<FMusicContext>& TmpContext) -> bool
	{
		return InKey == TmpContext->Key;
	});

	TSharedPtr<FMusicContext> TmpContext;
	if (!FindContext)
	{
		TmpContext = MakeShared<FMusicContext>();
		TmpContext->Key = InKey;
		
		MusicContexts.Add(TmpContext);
	}
}

bool FMusicContextContainer::StopContext(const FName& InKey)
{
	TSharedPtr<FMusicContext>* FindContext = MusicContexts.FindByPredicate([InKey](const TSharedPtr<FMusicContext>& TmpContext) -> bool
	{
		return InKey == TmpContext->Key;
	});

	if (FindContext && FindContext->IsValid())
	{
		(*FindContext)->Stop(false);
	}

	return IsActivate();
}

void FMusicContextContainer::DisplayInfos()
{
	UE_LOG(LogTemp, Warning, TEXT("\tMusicContextContainer Priority : %d, Activate : %d"),
    			GetPriority(), static_cast<int32>(IsActivate()));
	for (const auto& TmpContext : MusicContexts)
	{
		TmpContext->DisplayInfo();
	}
}

void UMyCheatManager::TestCheatInit()
{
	TestPlayNode(FName(TEXT("TEST1")), 1);
	TestPlayNode(FName(TEXT("TEST2")), 2);
	TestPlayNode(FName(TEXT("TEST3")), 3);
	TestPlayNode(FName(TEXT("TEST4")), 8);
	TestPlayNode(FName(TEXT("TEST5")), 5);
}

void UMyCheatManager::TestPlayNode(FName InKey, int32 Priority)
{
	UE_LOG(LogTemp, Warning, TEXT("TestPlayNode %s %d"), *InKey.ToString(), Priority);
	
	// 해당 키로 찾은다음에 있으면 Update만함
	TSharedPtr<FMusicContextContainer>* FindNode = MusicContextContainers.FindByPredicate([Priority](const TSharedPtr<FMusicContextContainer>& Node) -> bool
	{
		return Priority == Node->GetPriority();
	});

	TSharedPtr<FMusicContextContainer> TmpNodePtr = (FindNode)? *FindNode : nullptr;
	if (!TmpNodePtr.IsValid())
	{
		TmpNodePtr = MakeShared<FMusicContextContainer>(Priority);
		MusicContextContainers.Add(TmpNodePtr);
	}

	if (TmpNodePtr.IsValid())
	{
		bool bTmpPlay = ActivateTopPriorityMusicContainer.IsValid()? Priority >= ActivateTopPriorityMusicContainer.Pin()->GetPriority() : true;
		TmpNodePtr->AddContext(InKey, Priority);
		if (bTmpPlay)
		{
			TmpNodePtr->Play();
		}
	}
	
	UpdateNodeContext();
	DisplayNodeInfos();
}

void UMyCheatManager::TestStopNode(FName InKey, int32 InPriority)
{
	UE_LOG(LogTemp, Warning, TEXT("TestStopNode %s"), *InKey.ToString());
	
	TSharedPtr<FMusicContextContainer>* FindNode = MusicContextContainers.FindByPredicate([InPriority](const TSharedPtr<FMusicContextContainer>& TmpContainer) -> bool
   	{
   		return  InPriority == TmpContainer->GetPriority();
   	});

	if (FindNode && FindNode->IsValid())
   	{
		bool bTmpActivate = (*FindNode)->StopContext(InKey);
		if (bTmpActivate == false && ActivateTopPriorityMusicContainer.IsValid() && ActivateTopPriorityMusicContainer == (*FindNode))
		{
			// @NOTE: 다음 우선순위를 플레이
			if (MusicContextContainers.IsValidIndex(1) && MusicContextContainers[1].IsValid())
			{
				MusicContextContainers[1]->Play();
			}
		}
   	}
	
	UpdateNodeContext();
	DisplayNodeInfos();
}

void UMyCheatManager::TestReleaseMusic()
{
	for (const auto& TmpContainerPtr : MusicContextContainers)
	{
		TmpContainerPtr->StopAll(false);
	}
	MusicContextContainers.Reset();
	ActivateTopPriorityMusicContainer.Reset();

	DisplayNodeInfos();
}

void UMyCheatManager::DisplayNodeInfos()
{
	UE_LOG(LogTemp, Warning, TEXT("UMyCheatManager::DisplayNodeInfos"));

	for (const auto& TmpNode : MusicContextContainers)
	{
		TmpNode->DisplayInfos();
	}
}

void UMyCheatManager::SortMusicContainers()
{
	MusicContextContainers.Sort([](const TSharedPtr<FMusicContextContainer>& A, const TSharedPtr<FMusicContextContainer>& B) -> bool
	{
		if (!A.IsValid() || !B.IsValid())
   		{
			return false;
   		}

		if (A->IsActivate() == B->IsActivate())
		{
			return A->GetPriority() > B->GetPriority();
		}
		else
		{
			return A->GetPriority() > B->GetPriority() && A->IsActivate();
		}
	});
}

void UMyCheatManager::UpdateNodeContext()
{
	SortMusicContainers();

	if (MusicContextContainers.IsEmpty() || !MusicContextContainers[0].IsValid())
	{
		return;
	}
	
	if (!ActivateTopPriorityMusicContainer.IsValid())
	{
		// 1. 현재 활성화된게 없으면 현재 노드중 제일 높은 우선순위의 Node를 선정후 플레이한다.
		ActivateTopPriorityMusicContainer = MusicContextContainers[0];
		if (ActivateTopPriorityMusicContainer.IsValid())
		{
			ActivateTopPriorityMusicContainer.Pin()->Play();
		}
	}
	else if (ActivateTopPriorityMusicContainer != MusicContextContainers[0])
	{
		// 2. 우선순위가 더 높은 노드가 있으면 이전 노드은 Stop 시키고 현재 노드를 플레이한다.
		if (ActivateTopPriorityMusicContainer.Pin()->IsActivate())
		{
			// 새로 변경할 Contailer에 따라서 Stop 할지 Pause 할지를 정해야함.
			ActivateTopPriorityMusicContainer.Pin()->StopAll(MusicContextContainers[0]->GetPauseOrStopOnPlay());
		}
		ActivateTopPriorityMusicContainer = MusicContextContainers[0];
		ActivateTopPriorityMusicContainer.Pin()->Play();
	}
} 