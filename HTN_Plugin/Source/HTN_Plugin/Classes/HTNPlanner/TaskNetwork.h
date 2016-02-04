#pragma once

#include "HTNTask.h"
#include "TaskNetwork.generated.h"

class UHTNTask;
class UTaskNetwork;

USTRUCT(BlueprintType)
struct FTaskNetworkMemory
{
	GENERATED_BODY()

public:
	/** Instances of Task or Subnetwork Instances */
	TArray<TSharedPtr<FHTNTaskInstance>> SubNetworkInstances;

	/** If true, the Task will have to be executed or decomposed immediately when it no longer has any predecessors in the network. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Task Network Memory")
	uint8 bImmediate : 1;
	/** If true, the array of Sub-Network Instances is unordered and can be executed in any arbitrary order */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Task Network Memory")
	uint8 bUnordered : 1;
};

UCLASS(Blueprintable, HideDropdown)
class HTN_PLUGIN_API UTaskNetwork : public UHTNTask
{
	GENERATED_BODY()

public:
	UTaskNetwork(const FObjectInitializer& ObjectInitializer);

	/** Adds a new task or sub-network to the network. Used for construction of networks at runtime */
	void AddTaskOrSubNetwork(const TSharedPtr<FHTNTaskInstance>&, uint8* TaskMemory);

	/** Overridden to control whether the bImmediate and bUnordered properties can be edited */
	virtual bool CanEditChange(const UProperty* InProperty) const override;

	/** Cleans up the Task Network's memory */
	virtual void Cleanup(uint8* TaskMemory) override;

	/** Returns an array of all the task instances in the network (the returned array does not have any meaningful order) */
	TArray<TSharedPtr<FHTNTaskInstance>> CollectAllTasks(const uint8* TaskMemory) const;

	/** Creates and returns a deep copy of the entire Network */
	virtual TSharedPtr<FHTNTaskInstance> Copy(const TSharedPtr<FHTNTaskInstance>& OriginalInstance) override;

	/**
	 * Finds and returns an array of all Tasks that have no predecessors in the Task Network
	 * (can contain multiple Tasks that have equal priority).
	 */
	TArray<TSharedPtr<FHTNTaskInstance>> FindTasksWithoutPredecessors(const uint8* TaskMemory);

	float GetHeuristicCost(const TSharedPtr<struct FHTNWorldState>& WorldState, const uint8* TaskMemory) const;
	virtual uint16 GetInstanceMemorySize() const override;
	const TArray<TSubclassOf<UHTNTask>>& GetSubNetworks() const;
	const TArray<TSharedPtr<FHTNTaskInstance>>& GetSubNetworkInstances(const uint8* TaskMemory) const;

	/** Overridden to also instantiate sub-networks */
	virtual void Instantiate(UHTNPlannerComponent& HTNComp, uint8* TaskMemory) override;

	/**
	 * Returns true if this Task Network is atomic, and false otherwise.
	 *
	 * A Task Network is considered to be atomic if it only contains a single Task as ''sub-network'' 
	 * which is not of the type UTaskNetwork. Intuitively this means an atomic Task Network represents
	 * a single (primitive or compound) Task.
	 */
	bool IsAtomic(const uint8* TaskMemory) const;
	/** Returns true if this Task Network is empty (has no compound or primitive tasks anymore anywhere) */
	bool IsEmpty(const uint8* TaskMemory) const;
	/** Returns true if this Task Network is atomic and immediate (being immediate has no meaning if it's not atomic). */
	bool IsImmediate(const uint8* TaskMemory) const;
	/** Returns true if this Task Network is not atomic and unordered (being unordered has no meaning if it's atomic). */
	bool IsUnordered(const uint8* TaskMemory) const;

	/** Removes the given Task Instance */
	void Remove(const TSharedPtr<FHTNTaskInstance>& TaskInstance, uint8* TaskMemory);
	/** Removes the Task Instance at the given index */
	void Remove(int32 Index, uint8* TaskMemory);	// TO DO make this private?
	/** Replaces the given Task Instance with the given new sub-network */
	void Replace(const TSharedPtr<FHTNTaskInstance>& TaskInstance, const TSharedPtr<FHTNTaskInstance>& NewNetwork, uint8* TaskMemory);
	/** Replaces the Task Instance at the given index with the given new sub-network */
	void Replace(int32 Index, const TSharedPtr<FHTNTaskInstance>& NewNetwork, uint8* TaskMemory);	// TO DO make this private?

	/** Set whether the Network is immediate. Will produce undefined results if called on a non-atomic Network! */
	void SetImmediate(bool bStatus, uint8* TaskMemory);
	/** Set whether the Network is unordered. Will produce undefined results if called on an atomic Network! */
	void SetUnordered(bool bStatus, uint8* TaskMemory);

protected:
	/** 
	 * The sub-networks of this Hierarchical Task Network.
	 * A sub-network can either be another TaskNetwork (representing a lower level in the tree / network of Tasks), 
	 * or a PrimitiveTask or CompoundTask (representing a Task that needs to be executed or decomposed into Tasks that need executing)
	 */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network")
	TArray<TSubclassOf<UHTNTask>> SubNetworks;

	/** Can be used to write some comments to explain the Network. Will not be used in-game. */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network Comments", meta=(MultiLine=true))
	FString NetworkComments;

	/** If true, the Task will have to be executed or decomposed immediately when it no longer has any predecessors in the network. */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network")
	uint8 bImmediate : 1;
	/** If true, the array of Sub-Networks is unordered and can be executed in any arbitrary order */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network")
	uint8 bUnordered : 1;

private:
	/**
	 * Recursively called (initially by Remove() and Replace() to find out which sub-network
	 * contains the given Task Instance at which index.
	 *
	 * When the first call to this method finishes, ContainingNetwork should point to the sub-network
	 * that contains the given Task Instance, and Index should contain the index at which that sub-network
	 * contains the given Task Instance.
	 *
	 * This method uses the same rules for priority as FindTasksWithoutPredecessors.
	 */
	bool FindTaskInstance(const TSharedPtr<FHTNTaskInstance>& TaskInstance, UTaskNetwork*& ContainingNetwork,
						  uint8*& ContainingNetworkMemory, int32& Index, uint8* TaskMemory);

	/**
	 * Recursively called (initially by FindTasksWithoutPredecessors() to collect all Task Instances
	 * without predecessors in the network.
	 *
	 * Returns true if a Task flagged as immediate was found, and false otherwise.
	 * Places all Tasks found in the given OutTasks array.
	 */
	bool FindTasksWithoutPredecessorsRecursion(TArray<TSharedPtr<FHTNTaskInstance>>& OutTasks, const uint8* TaskMemory);
};