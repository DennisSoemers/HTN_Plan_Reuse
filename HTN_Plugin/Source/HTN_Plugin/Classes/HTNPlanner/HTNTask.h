#pragma once

#include "HTNPlanner/HTNPlannerTypes.h"
#include "HTNTask.generated.h"

class UHTNPlannerComponent;

/**
 * Base class for Tasks in a Hierarchical Task Network (HTN).
 *
 * Can be either a primitive Task (a Task that can be executed by an agent directly) or a compound Task
 * (a list of other Tasks that need to be executed in order to consider the compound Task executed).
 */
UCLASS(Abstract)
class HTN_PLUGIN_API UHTNTask : public UObject
{
	GENERATED_BODY()

public:
	virtual void Cleanup(uint8* TaskMemory);

	/**
	 * This will be called whenever a deep copy of a Task Instance is considered to be necessary to guarantee
	 * a correct planning process.
	 *
	 * The default implementation will simply return the given Task Instance instead of actually creating a copy.
	 *
	 * In cases where a Task has internal memory that is important and can change during the planning process, 
	 * the function should be overridden to correctly create a real copy.
	 */
	virtual TSharedPtr<FHTNTaskInstance> Copy(const TSharedPtr<FHTNTaskInstance>& OriginalInstance);

	/** size of instance memory */
	virtual uint16 GetInstanceMemorySize() const;

	/** Returns the name of this Task */
	virtual FString GetTaskName(uint8* TaskMemory) const;

	/** 
	 * Will be called when creating instances through the static Instantiate() method. 
	 * Can be used for instantiating sub-tasks.
	 */
	virtual void Instantiate(UHTNPlannerComponent& HTNComp, uint8* TaskMemory);

protected:
	/** Name used in logs and debugging messages to describe this Task or Task Network */
	UPROPERTY(EditAnywhere, Category = "Hierarchical Task Network", meta=(DisplayName="Name"))
	FString TaskName;
};