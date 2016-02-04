#pragma once

#include "BrainComponent.h"
#include "HTNPlannerTypes.generated.h"

class UBlackboardKeyType;
class UHTNTask;

// Since this enum is equivalent to EBTNodeResult in BehaviorTreeTypes.h, it would be cool if this was
// changed to be not specific to either type of BrainComponent
UENUM(BlueprintType)
enum class EHTNExecutionResult : uint8
{
	Succeeded,		// finished as success
	Failed,			// finished as failure
	Aborted,		// finished aborting = failure
	InProgress,		// not finished yet
};

// Since this enum is equivalent to EBTTaskStatus in BehaviorTreeTypes.h, it would be cool if this was
// changed to be not specific to either type of BrainComponent
enum class EHTNTaskStatus : uint8
{
	Active,
	Aborting,
	Inactive,
};

// Since this enum is equivalent to the one in BehaviorTreeTypes.h, it would be cool if this was
// changed to be not specific to either type of BrainComponent
enum class EHTNStopMode : uint8
{
	Safe,
	Forced
};

/**
 * Struct used to keep track of instances of Tasks during the planning process.
 * Contains a pointer to a Task object (generally the CDO) for calling functions,
 * and a pointer to the Task's memory specific to that instance.
 */
USTRUCT(BlueprintType)
struct FHTNTaskInstance
{
	GENERATED_BODY()

private:
	/** Memory of the specific Task instance */
	TArray<uint8> TaskMemory;

public:
	/** Task object (generally CDO) that we can call functions on */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Task Instance")
	class UHTNTask* Task;

	~FHTNTaskInstance();

	// TO DO keep track of memory stats (see BehaviorTreeTypes.h FBehaviorTreeInstance)?
	FHTNTaskInstance() : Task(nullptr) {}
	FHTNTaskInstance(class UHTNTask* Task, int32 MemorySize) 
	{ 
		this->Task = Task;
		TaskMemory.AddZeroed(MemorySize);
	}

	FORCEINLINE uint8* GetMemory() { return TaskMemory.GetData(); }
	FORCEINLINE const TArray<uint8>& GetUint8Memory() const { return TaskMemory; }
};

UCLASS(Abstract)
class HTN_PLUGIN_API UHTNPlannerTypes : public UObject
{
	GENERATED_BODY()

public:
	static FString DescribeTaskHelper(const UHTNTask* Task, uint8* TaskMemory);

	static FString DescribeTaskResult(EHTNExecutionResult TaskResult);
};