#include "HTN_PluginPrivatePCH.h"
#include "HTNPlanner/HTNPlannerTypes.h"
#include "HTNPlanner/CompoundTask.h"
#include "HTNPlanner/HTNWorldState.h"
#include "HTNPlanner/PrimitiveTask.h"
#include "HTNPlanner/TaskNetwork.h"

UTaskNetwork::UTaskNetwork(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bImmediate = false;
	bUnordered = false;
}

void UTaskNetwork::AddTaskOrSubNetwork(const TSharedPtr<FHTNTaskInstance>& TaskOrSubNetwork, uint8* TaskMemory)
{
	if(FTaskNetworkMemory* NetworkMemory = (FTaskNetworkMemory*)TaskMemory)
	{
		NetworkMemory->SubNetworkInstances.Add(TaskOrSubNetwork);
	}
	else
	{
		UE_LOG(LogHTNPlanner, Warning, TEXT("UTaskNetwork::AddTaskOrSubNetwork should not be called on a non-instanced network!"));
		return;
	}
}

bool UTaskNetwork::CanEditChange(const UProperty* InProperty) const
{
	if(!Super::CanEditChange(InProperty))
	{
		return false;
	}

	if(InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UTaskNetwork, bImmediate))
	{
		// can only edit the bImmediate property if we're atomic
		return IsAtomic(nullptr);
	}
	else if(InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UTaskNetwork, bUnordered))
	{
		// can only edit the bUnordered property if we're not atomic
		return !IsAtomic(nullptr);
	}

	return true;
}

void UTaskNetwork::Cleanup(uint8* TaskMemory)
{
	if(TaskMemory)
	{
		FTaskNetworkMemory* Memory = (FTaskNetworkMemory*)TaskMemory;
		Memory->SubNetworkInstances.Empty();
	}
}

TArray<TSharedPtr<FHTNTaskInstance>> UTaskNetwork::CollectAllTasks(const uint8* TaskMemory) const
{
	const FTaskNetworkMemory* NetworkMemory = (FTaskNetworkMemory*)TaskMemory;
	TArray<TSharedPtr<FHTNTaskInstance>> Tasks;

	for(int32 Idx = 0; Idx < NetworkMemory->SubNetworkInstances.Num(); ++Idx)
	{
		TSharedPtr<FHTNTaskInstance> SubInstance = NetworkMemory->SubNetworkInstances[Idx];
		UHTNTask* SubNetwork = SubInstance->Task;

		if(UTaskNetwork* SubTaskNetwork = Cast<UTaskNetwork>(SubNetwork))
		{
			Tasks.Append(SubTaskNetwork->CollectAllTasks(SubInstance->GetMemory()));
		}
		else
		{
			Tasks.Add(SubInstance);
		}
	}

	return Tasks;
}

TSharedPtr<FHTNTaskInstance> UTaskNetwork::Copy(const TSharedPtr<FHTNTaskInstance>& OriginalInstance)
{
	TSharedPtr<FHTNTaskInstance> NewInstance = TSharedPtr<FHTNTaskInstance>(new FHTNTaskInstance(OriginalInstance->Task, GetInstanceMemorySize()));
	
	FTaskNetworkMemory* NewInstanceMemory = (FTaskNetworkMemory*)NewInstance->GetMemory();
	NewInstanceMemory->bImmediate = IsImmediate(OriginalInstance->GetMemory());
	NewInstanceMemory->bUnordered = IsUnordered(OriginalInstance->GetMemory());
	NewInstanceMemory->SubNetworkInstances = TArray<TSharedPtr<FHTNTaskInstance>>();

	FTaskNetworkMemory* OriginalInstanceMemory = (FTaskNetworkMemory*)OriginalInstance->GetMemory();
	const TArray<TSharedPtr<FHTNTaskInstance>>& OriginalSubNetworkInstances = OriginalInstanceMemory->SubNetworkInstances;
	TArray<TSharedPtr<FHTNTaskInstance>>& NewSubNetworkInstances = NewInstanceMemory->SubNetworkInstances;
	NewSubNetworkInstances.Reserve(OriginalSubNetworkInstances.Num());

	for(int32 Idx = 0; Idx < OriginalSubNetworkInstances.Num(); ++Idx)
	{
		UHTNTask* OriginalSubNetworkTask = OriginalSubNetworkInstances[Idx]->Task;

		if(OriginalSubNetworkTask->IsA(UTaskNetwork::StaticClass()))
		{
			NewSubNetworkInstances.Add(OriginalSubNetworkTask->Copy(OriginalSubNetworkInstances[Idx]));
		}
		else
		{
			// TO DO probably this copy shouldn't be necessary, but then we should enforce that
			// all tasks are const during planning process in HTNPlannerComponent
			NewSubNetworkInstances.Add(OriginalSubNetworkTask->Copy(OriginalSubNetworkInstances[Idx]));
		}
	}

	return NewInstance;
}

bool UTaskNetwork::FindTaskInstance(const TSharedPtr<FHTNTaskInstance>& TaskInstance, UTaskNetwork*& ContainingNetwork,
									uint8*& ContainingNetworkMemory, int32& Index, uint8* TaskMemory)
{
	const FTaskNetworkMemory* NetworkMemory = (const FTaskNetworkMemory*)TaskMemory;

	if(IsAtomic(TaskMemory))
	{
		if(TaskInstance == NetworkMemory->SubNetworkInstances[0])	// we've found a match
		{
			if(bImmediate)	// task is immediate, so this must be our final match
			{
				ContainingNetwork = this;
				ContainingNetworkMemory = TaskMemory;
				Index = 0;
				return true;
			}
			else if(Index < 0)
			{
				// match is not immediate, but we didn't find a match yet, so take this one
				ContainingNetwork = this;
				ContainingNetworkMemory = TaskMemory;
				Index = 0;
			}

			return false;
		}
		else if(bImmediate)
		{
			// hit a non-matching immediate task
			return true;
		}
		else
		{
			return false;
		}
	}
	else if(!IsUnordered(TaskMemory))	// we have an ordered list of tasks / task networks
	{
		for(int32 Idx = 0; Idx < NetworkMemory->SubNetworkInstances.Num(); ++Idx)
		{
			TSharedPtr<FHTNTaskInstance> SubInstance = NetworkMemory->SubNetworkInstances[Idx];
			UHTNTask* SubNetwork = SubInstance->Task;

			if(UTaskNetwork* Network = Cast<UTaskNetwork>(SubNetwork))
			{
				// a true sub-network, so recursively call on that
				if(Network->FindTaskInstance(TaskInstance, ContainingNetwork, ContainingNetworkMemory, Index, SubInstance->GetMemory()))
				{
					// recursive call returned true and therefore found an immediate task match, so we'll also return
					return true;
				}
				else if(!Network->IsEmpty(SubInstance->GetMemory()))
				{
					// the subnetwork was not empty, so since we're ordered we shouldn't look at further indices
					return false;
				}
			}
			else
			{
				// we've found a sub-network that's actually just a single task
				if(TaskInstance == SubInstance && Index < 0)
				{
					// the Task is a match and we didn't have a match yet
					ContainingNetwork = this;
					ContainingNetworkMemory = TaskMemory;
					Index = Idx;
					return false;	// it was not immediate, so return false
				}
				else if(SubNetwork != nullptr)
				{
					// there was a valid (non-null) task, so since we're an ordered list we have to return
					return false;
				}
			}
		}

		return false;
	}
	else	// we have an unordered list of tasks / task networks
	{
		for(int32 Idx = 0; Idx < NetworkMemory->SubNetworkInstances.Num(); ++Idx)
		{
			TSharedPtr<FHTNTaskInstance> SubInstance = NetworkMemory->SubNetworkInstances[Idx];
			UHTNTask* SubNetwork = SubInstance->Task;

			if(UTaskNetwork* Network = Cast<UTaskNetwork>(SubNetwork))
			{
				// a true sub-network, so recursively call on that
				if(Network->FindTaskInstance(TaskInstance, ContainingNetwork, ContainingNetworkMemory, Index, SubInstance->GetMemory()))
				{
					// recursive call returned true and therefore found an immediate task match, so we'll also return
					return true;
				}
				// else if no immediate task match found, we'll simply continue because we're unordered
			}
			else
			{
				// we've found a sub-network that's actually just a single task
				if(TaskInstance == SubInstance && Index < 0)
				{
					// the Task is a match and we didn't have a match yet
					ContainingNetwork = this;
					ContainingNetworkMemory = TaskMemory;
					Index = Idx;
					return false;	// it was not immediate, so return false
				}
			}
		}

		return false;
	}
}

TArray<TSharedPtr<FHTNTaskInstance>> UTaskNetwork::FindTasksWithoutPredecessors(const uint8* TaskMemory)
{
	TArray<TSharedPtr<FHTNTaskInstance>> Tasks = TArray<TSharedPtr<FHTNTaskInstance>>();
	FindTasksWithoutPredecessorsRecursion(Tasks, TaskMemory);
	return Tasks;
}

bool UTaskNetwork::FindTasksWithoutPredecessorsRecursion(TArray<TSharedPtr<FHTNTaskInstance>>& OutTasks, const uint8* TaskMemory)
{
	const FTaskNetworkMemory* NetworkMemory = (const FTaskNetworkMemory*)TaskMemory;

	if(IsAtomic(TaskMemory))
	{
		if(IsImmediate(TaskMemory))	// we've run into an immediate task, so we want to return only this one
		{
			OutTasks.Empty(1);
			OutTasks.Add(NetworkMemory->SubNetworkInstances[0]);
			return true;
		}

		// we have a non-immediate atomic task, so we add it and continue searching for equal-priority tasks
		OutTasks.Add(NetworkMemory->SubNetworkInstances[0]);
		return false;
	}
	else if(!IsUnordered(TaskMemory))	// we have an ordered list of tasks / task networks
	{
		int32 PreviousSize = OutTasks.Num();

		for(int32 Idx = 0; Idx < NetworkMemory->SubNetworkInstances.Num(); ++Idx)
		{
			TSharedPtr<FHTNTaskInstance> SubInstance = NetworkMemory->SubNetworkInstances[Idx];
			UHTNTask* SubNetwork = SubInstance->Task;

			if(SubNetwork->IsA(UTaskNetwork::StaticClass()))
			{
				// a true sub-network, so recursively call on that
				if(Cast<UTaskNetwork>(SubNetwork)->FindTasksWithoutPredecessorsRecursion(OutTasks, SubInstance->GetMemory()))
				{
					// recursive call returned true and therefore found an immediate task, so we'll also return
					return true;
				}
				else if(OutTasks.Num() != PreviousSize)
				{
					// recursive call did not return true, so did not find immediate task. OutTasks size changed,
					// so it must have found some task though, and since we're an ordered list, we'll stop
					return false;
				}
			}
			else
			{
				// we've found a sub-network that's actually just a single task
				OutTasks.Add(SubInstance);
				return false;	// cannot be marked as immediate
			}
		}

		return false;
	}
	else	// we have an unordered list of tasks / task networks
	{
		for(int32 Idx = 0; Idx < NetworkMemory->SubNetworkInstances.Num(); ++Idx)
		{
			TSharedPtr<FHTNTaskInstance> SubInstance = NetworkMemory->SubNetworkInstances[Idx];
			UHTNTask* SubNetwork = SubInstance->Task;

			if(SubNetwork->IsA(UTaskNetwork::StaticClass()))
			{
				// a true sub-network, so recursively call on that
				if(Cast<UTaskNetwork>(SubNetwork)->FindTasksWithoutPredecessorsRecursion(OutTasks, SubInstance->GetMemory()))
				{
					// recursive call returned true and therefore found an immediate task, so we'll also return
					return true;
				}
				// else if no immediate task found, we'll simply continue because we're unordered and can add more than one task
			}
			else
			{
				// we've found a sub-network that's actually just a single task
				OutTasks.Add(SubInstance);
			}
		}

		return false;
	}
}

float UTaskNetwork::GetHeuristicCost(const TSharedPtr<FHTNWorldState>& WorldState, const uint8* TaskMemory) const
{
	const FTaskNetworkMemory* NetworkMemory = (const FTaskNetworkMemory*)TaskMemory;
	float HeuristicCost = 0.f;

	TArray<TSharedPtr<FHTNTaskInstance>> TaskInstances = CollectAllTasks(TaskMemory);

	for(int32 Idx = 0; Idx < TaskInstances.Num(); ++Idx)
	{
		TSharedPtr<FHTNTaskInstance> TaskInstance = TaskInstances[Idx];
		UHTNTask* Task = TaskInstance->Task;

		if(UCompoundTask* CompoundTask = Cast<UCompoundTask>(Task))
		{
			HeuristicCost += CompoundTask->GetHeuristicCost();
		}
		else if(UPrimitiveTask* PrimitiveTask = Cast<UPrimitiveTask>(Task))
		{
			HeuristicCost += PrimitiveTask->GetHeuristicCost();
		}
	}

	HeuristicCost += WorldState->GetHeuristicCost(TaskInstances);

	return HeuristicCost;
}

uint16 UTaskNetwork::GetInstanceMemorySize() const
{
	return sizeof(FTaskNetworkMemory);
}

const TArray<TSubclassOf<UHTNTask>>& UTaskNetwork::GetSubNetworks() const
{
	return SubNetworks;
}

const TArray<TSharedPtr<FHTNTaskInstance>>& UTaskNetwork::GetSubNetworkInstances(const uint8* TaskMemory) const
{
	return ((const FTaskNetworkMemory*)TaskMemory)->SubNetworkInstances;
}

void UTaskNetwork::Instantiate(UHTNPlannerComponent& HTNComp, uint8* TaskMemory)
{
	FTaskNetworkMemory* NetworkMemory = (FTaskNetworkMemory*)TaskMemory;
	NetworkMemory->SubNetworkInstances = TArray<TSharedPtr<FHTNTaskInstance>>();
	TArray<TSharedPtr<FHTNTaskInstance>>& SubNetworkInstances = NetworkMemory->SubNetworkInstances;

	// instantiate sub-networks
	SubNetworkInstances.Reserve(SubNetworks.Num());
	for(int32 Idx = 0; Idx < SubNetworks.Num(); ++Idx)
	{
		TSharedPtr<FHTNTaskInstance> Instance;

		if(SubNetworks[Idx]->IsChildOf(UTaskNetwork::StaticClass()))
		{
			TSubclassOf<UTaskNetwork> NetworkSubclass = *(SubNetworks[Idx]);
			Instance = HTNComp.InstantiateNetwork(NetworkSubclass);
		}
		else
		{
			Instance = HTNComp.InstantiateTask(SubNetworks[Idx]);
		}

		if(Instance.IsValid())
		{
			SubNetworkInstances.Add(Instance);
		}
	}

	// because the IsAtomic() and IsImmediate() functions are gonna change their behavior after
	// instancing, we're gonna have to make sure here that the bImmediate and bUnordered flags
	// are not illegally set to true (which is possible if they're set to true in-editor first,
	// and the user changed SubNetworks in a way that makes the properties uneditable afterwards)
	if(IsAtomic(TaskMemory))
	{
		bUnordered = false;
	}
	else
	{
		bImmediate = false;
	}
}

bool UTaskNetwork::IsAtomic(const uint8* TaskMemory) const
{
	if(TaskMemory)
	{
		const FTaskNetworkMemory* NetworkMemory = (const FTaskNetworkMemory*)TaskMemory;

		if(NetworkMemory->SubNetworkInstances.Num() != 1)
		{
			// we need to have exactly one sub-network to be considered atomic
			return false;
		}
		else if(NetworkMemory->SubNetworkInstances[0]->Task->IsA(UTaskNetwork::StaticClass()))
		{
			// we have a real Task Network as sub-network, not just a task
			return false;
		}
	}
	else
	{
		if(SubNetworks.Num() != 1)
		{
			// we need to have exactly one sub-network to be considered atomic
			return false;
		}
		else if(*SubNetworks[0] == nullptr || SubNetworks[0]->IsChildOf(UTaskNetwork::StaticClass()))
		{
			// here we know there is exactly one sub-network
			// if the class evaluates to a nullptr, it has not been set yet in-editor and we'll assume for now that it won't be atomic
			// if it doesn't evaluate to nullptr, but it is a UTaskNetwork, we still won't be atomic
			return false;
		}
	}

	// must be atomic if we didn't return yet
	return true;
}

bool UTaskNetwork::IsEmpty(const uint8* TaskMemory) const
{
	if(TaskMemory)
	{
		const FTaskNetworkMemory* NetworkMemory = (const FTaskNetworkMemory*)TaskMemory;

		if(NetworkMemory->SubNetworkInstances.Num() == 0)
		{
			return true;
		}
		else
		{
			for(const TSharedPtr<FHTNTaskInstance>& TaskInstance : NetworkMemory->SubNetworkInstances)
			{
				UHTNTask* Task = TaskInstance->Task;

				if(const UTaskNetwork* SubNetwork = Cast<UTaskNetwork>(Task))
				{
					// we have a real Task Network as sub-network, so we need it to be empty for us to be empty
					if(!SubNetwork->IsEmpty(TaskInstance->GetMemory()))
					{
						return false;
					}
				}
				else
				{
					// we have a primitive or compound task as ''sub-network'', so we're not empty
					return false;
				}
			}

			// we didn't return false yet, so we must be empty
			return true;
		}
	}
	else
	{
		// we currently don't have any particular meaning for the concept of a non-instanced Task Network being empty
		UE_LOG(LogHTNPlanner, Warning, TEXT("UTaskNetwork::IsEmpty() has no meaning when called on a non-instanced Task Network!!"))
		return false;
	}
}

bool UTaskNetwork::IsImmediate(const uint8* TaskMemory) const
{
	return IsAtomic(TaskMemory) && bImmediate;
}

bool UTaskNetwork::IsUnordered(const uint8* TaskMemory) const
{
	return !IsAtomic(TaskMemory) && bUnordered;
}

void UTaskNetwork::Remove(const TSharedPtr<FHTNTaskInstance>& TaskInstance, uint8* TaskMemory)
{
	UTaskNetwork* ContainingNetwork = nullptr;
	uint8* ContainingNetworkMemory = nullptr;
	int32 Index = -1;
	FindTaskInstance(TaskInstance, ContainingNetwork, ContainingNetworkMemory, Index, TaskMemory);

	if(ContainingNetwork != nullptr)
	{
		ContainingNetwork->Remove(Index, ContainingNetworkMemory);
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UTaskNetwork::Remove() was not able to find a Network that contained the given Task Instance!"));
	}
}

void UTaskNetwork::Remove(int32 Index, uint8* TaskMemory)
{
	FTaskNetworkMemory* NetworkMemory = (FTaskNetworkMemory*)TaskMemory;
	NetworkMemory->SubNetworkInstances.RemoveAt(Index, 1);
}

void UTaskNetwork::Replace(const TSharedPtr<FHTNTaskInstance>& TaskInstance, const TSharedPtr<FHTNTaskInstance>& NewNetwork, uint8* TaskMemory)
{
	UTaskNetwork* ContainingNetwork = nullptr;
	uint8* ContainingNetworkMemory = nullptr;
	int32 Index = -1;
	FindTaskInstance(TaskInstance, ContainingNetwork, ContainingNetworkMemory, Index, TaskMemory);

	if(ContainingNetwork != nullptr)
	{
		ContainingNetwork->Replace(Index, NewNetwork, ContainingNetworkMemory);
	}
	else
	{
		UE_LOG(LogHTNPlanner, Error, TEXT("UTaskNetwork::Replace() was not able to find a Network that contained the given Task Instance!"));
	}
}

void UTaskNetwork::Replace(int32 Index, const TSharedPtr<FHTNTaskInstance>& NewNetwork, uint8* TaskMemory)
{
	FTaskNetworkMemory* NetworkMemory = (FTaskNetworkMemory*)TaskMemory;
	NetworkMemory->SubNetworkInstances[Index] = TSharedPtr<FHTNTaskInstance>(NewNetwork);
}

void UTaskNetwork::SetImmediate(bool bStatus, uint8* TaskMemory)
{
	FTaskNetworkMemory* NetworkMemory = (FTaskNetworkMemory*)TaskMemory;
	NetworkMemory->bImmediate = bStatus;
}

void UTaskNetwork::SetUnordered(bool bStatus, uint8* TaskMemory)
{
	FTaskNetworkMemory* NetworkMemory = (FTaskNetworkMemory*)TaskMemory;
	NetworkMemory->bUnordered = bStatus;
}