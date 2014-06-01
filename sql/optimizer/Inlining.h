/**********************************************************************
// @@@ START COPYRIGHT @@@
//
// (C) Copyright 2003-2014 Hewlett-Packard Development Company, L.P.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
// @@@ END COPYRIGHT @@@
**********************************************************************/
#ifndef INLINING_H
#define INLINING_H

/* -*-C++-*-
******************************************************************************
*
* File:         Inlining.h
* Description:  Definition of class InliningInfo.
*
* Created:      6/23/98
* Language:     C++
* Status:       $State: Exp $
*
*
******************************************************************************
*/

#include "ComSmallDefs.h"
#include "ValueDesc.h"
#include "ItemFunc.h"

// forward reference
#include "CostScalar.h"
class Trigger;
class TriggerBindInfo;
class TableDesc;
class BindWA;
class NormWA;
class ForceCardinalityInfo;

//////////////////////////////////////////////////////////////////////////////
// An InliningInfo object is contained in each and every RelExpr.
// flags_ is a bitmap of flags marking what is the node used for in the trigger
// inlining tree.
// triggerObject_ is a pointer to the Trigger object, from which information
// such as timestamp can be retrieved. Valid only for "TriggerRoot" nodes.
// This information is set up during binding by the methods in Inlining.cpp,
// and used by the trigger transformation code in OptTrigger.cpp and the
// Access sets code in AccessSets.cpp.
//////////////////////////////////////////////////////////////////////////////

enum InliningInfoEnum {
  // this TSJ's child(1) is a temp Insert.
  II_DrivingTempInsert		= 0x00000001, 
  // this TSJ's child(1) is a tree of row triggers and RI.
  II_DrivingPipelinedActions	= 0x00000002,
  // this TSJ's child(1) is a tree of row triggers.
  II_DrivingRowTrigger		= 0x00000004,
  // this Ordered Union's child(1) is a tree of statement triggers.
  II_DrivingStatementTrigger	= 0x00000008,
  // this Ordered Union's child(1) is a temp Delete.
  II_DrivingTempDelete		= 0x00000010,
  // this Union has before triggers on child(0), after triggers on child(1).
  II_DrivingBeforeTriggers	= 0x00000020,
  // this Union's child(0) is an RI tree.
  II_DrivingRI			= 0x00000040,
  // this TSJ's child(1) is an IM tree.
  II_DrivingIM			= 0x00000080,
  // this Union is the IF node of a trigger root.
  II_TriggerRoot		= 0x00000100,
  // located on a DrivingTempDelete node, when before triggers are present.
  II_BeforeTriggersExist	= 0x00000200,
  // Access Sets should be saved in this node.
  II_AccessSetNeeded		= 0x00000400,
  // this Tuple node should be removed during transformation.
  II_DummyStatement		= 0x00000800,
  // this EffectiveGU has also row triggers, RI or IM to
  // pipeline values to.
  II_hasPipelinedActions	= 0x00001000,
  // this GenericUpdate is the Effective GU of a before trigger. 
  // Do not do inlining again.
  II_EffectiveGU		= 0x00002000,
  // This RelRoot node is on top of the RI action.
  II_ActionOfRI			= 0x00004000,
  // avoid security checks
  II_AvoidSecurityChecks		= 0x00008000,
  // Does the outputs of this GU go to an MV log (CurrentEpoch required)?
  II_isMVLoggingInlined		= 0x00010000,
  // Marks the MV log insert TSJ for the push-to-DP2 transformation rule.
  II_DrivingMvLogInsert		= 0x00020000,
  // VSBB Insert drives MV range logging and also IM etc.
  II_ProjectMidRangeRows        = 0x00040000,
  // Need to project the OLD/NEW outputs from the GU node, for other purposes.
  II_NeedGuOutputs              = 0x00080000,
  // Don't apply join commutativity rule
  II_JoinChildsOrder		= 0x00100000,
  // Force MDAM on the table
  II_MdamForcedByInlining	= 0x00200000,
  // Force Single execution for TSJ
  II_SingleExecutionForTSJ	= 0x00400000,
  // Enable the use of First N Rows for Mvlog
  II_EnableFirstNRows		= 0x00800000,
  // Do not implement using the Materialize node
  II_MaterializeNodeForbidden	= 0x01000000,
  // this GU has IM
  II_hasIM	                = 0x02000000,
  // this GU has some inlined actions(IM, RI, triggers, MV logging...)
  II_hasInlinedActions          = 0x04000000,
  // this GU has RI
  II_hasRI                      = 0x08000000,
  // this GU has Triggers
  II_hasTriggers                = 0x10000000,
  // Force Single execution for TSJ driving row Triggers
  II_SingleExecutionForTriggersTSJ	= 0x20000000,
  // this update operator is part of an action of a row trigger
  II_InActionOfRowTrigger     	= 0x40000000,
  // Is this RelRoot, Union, or Insert used in MV logging -- ONLY USED FOR PUSH DOWN
  II_isUsedForMvLogging         = 0x80000000
  
};

class InliningInfo
{
public:
  // Default constructor
  InliningInfo()
    : flags_(0),
      triggerObject_(NULL),
      forceCardinalityInfo_(NULL),
      triggerBindInfo_(NULL)
  {};

  virtual ~InliningInfo();

  // Accessors
  inline NABoolean 
  isDrivingTempInsert()       const { return forceBool(II_DrivingTempInsert); }
  inline NABoolean 
  isDrivingPipelinedActions() const { return forceBool(II_DrivingPipelinedActions); }
  inline NABoolean 
  isDrivingRowTrigger()       const { return forceBool(II_DrivingRowTrigger); }
  inline NABoolean
  isDrivingStatementTrigger() const { return forceBool(II_DrivingStatementTrigger); }
  inline NABoolean 
  isDrivingTempDelete()       const { return forceBool(II_DrivingTempDelete); }
  inline NABoolean 
  isDrivingBeforeTriggers()   const { return forceBool(II_DrivingBeforeTriggers); }
  inline NABoolean 
  isDrivingRI()               const { return forceBool(II_DrivingRI); }
  inline NABoolean 
  isDrivingIM()               const { return forceBool(II_DrivingIM); }
  inline NABoolean 
  isTriggerRoot()             const { return forceBool(II_TriggerRoot); }
  inline NABoolean 
  isBeforeTriggersExist()     const { return forceBool(II_BeforeTriggersExist); }
  inline NABoolean 
  isAccessSetNeeded()         const { return forceBool(II_AccessSetNeeded); }
  inline NABoolean 
  isDummy()		      const { return forceBool(II_DummyStatement); }
  inline NABoolean 
  hasPipelinedActions()	      const { return forceBool(II_hasPipelinedActions); }
  inline NABoolean 
  isEffectiveGU()	      const { return forceBool(II_EffectiveGU); }
  inline NABoolean 
  isActionOfRI()	      const { return forceBool(II_ActionOfRI); }
  inline NABoolean 
  isAvoidSecurityCheck()	      const { return forceBool(II_AvoidSecurityChecks); }
  inline NABoolean
  isMVLoggingInlined()	      const { return forceBool(II_isMVLoggingInlined); }
  inline NABoolean
  isDrivingMvLogInsert()      const { return forceBool(II_DrivingMvLogInsert); }
  inline NABoolean
  isProjectMidRangeRows()     const { return forceBool(II_ProjectMidRangeRows); }
  inline NABoolean
  isNeedGuOutputs()           const { return forceBool(II_NeedGuOutputs); }
  inline NABoolean
  isJoinOrderForcedByInlining() const { return forceBool(II_JoinChildsOrder); }
  inline NABoolean
  isMdamForcedByInlining()    const { return forceBool(II_MdamForcedByInlining); }
  inline NABoolean
  isSingleExecutionForTSJ()    const { return forceBool(II_SingleExecutionForTSJ); }
  inline NABoolean
  isSingleExecutionForTriggersTSJ()    const { return forceBool(II_SingleExecutionForTriggersTSJ); }
  inline NABoolean
  isInActionOfRowTrigger()    const { return forceBool(II_InActionOfRowTrigger); }
  inline NABoolean
  isEnableFirstNRows()        const { return forceBool(II_EnableFirstNRows); }
  inline NABoolean
  isMaterializeNodeForbidden() const { return forceBool(II_MaterializeNodeForbidden); }
  inline NABoolean 
  hasIM()	      const { return forceBool(II_hasIM); }
  inline NABoolean 
  hasRI()	      const { return forceBool(II_hasRI); }
  inline NABoolean 
  hasTriggers()	      const { return forceBool(II_hasTriggers); }
  inline NABoolean 
  hasInlinedActions()	      const { return forceBool(II_hasInlinedActions); }
  inline NABoolean
  isForceCardinality()	      const { return forceCardinalityInfo_ != NULL; }
  inline NABoolean
  isUsedForMvLogging()	      const { return forceBool(II_isUsedForMvLogging); }


  inline NABoolean
  isTopOfTriggerBackbone()    const 
  {
    return ( isDrivingBeforeTriggers() ||
	     (isDrivingTempDelete() && !isDrivingBeforeTriggers() ) );
  }

  inline NABoolean isSystemGenerated() const { return flags_ ? TRUE: FALSE; }

  inline Trigger *getTriggerObject() const { return triggerObject_;}
  inline TriggerBindInfo *getTriggerBindInfo() const { return triggerBindInfo_; }

  // getNewCardinality() returns the value of  cardinality * cardinalityFactor_ .
  // The cardinality is taken from either cardinality_ or when cardinality_ is zero
  // from oldCardinality parameter.
  CostScalar getNewCardinality(CostScalar oldCardinality) const;

  static const char *getExecIdVirtualColName()   { return execIdVirtualColName_; }
  static const char *getEpochVirtualColName()    { return epochVirtualColName_; }
  static const char *getMvLogTsColName()         { return mvLogTsColName_; }

  // LCOV_EXCL_START
  // only used for range logging which is a not supported feature
  static const char *getRowTypeVirtualColName()  { return rowtypeVirtualColName_; }
  static const char *getRowCountVirtualColName() { return rowcountVirtualColName_; }
  // LCOV_EXCL_STOP
  
  // Mutators
  inline void setFlags(Int32 flags)                {flags_ |= flags;}
  inline void resetFlags(Int32 flags)              {flags_ &= ~flags;}
  inline void setTriggerObject(Trigger *trigger) {triggerObject_ = trigger;}

  void merge(InliningInfo *other);

  void BuildForceCardinalityInfo(double cardinalityFactor, 
				 Cardinality cardinality, 
				 CollHeap *heap);
  
  void buildTriggerBindInfo(BindWA *bindWA,
			    RETDesc *RETDesc,
			    CollHeap *heap);
  
private:
  // This method forces the result to FALSE or TRUE
  // instead of zero and non-zero.
  inline NABoolean forceBool(InliningInfoEnum bitToTest) const
    { return (flags_ & bitToTest) ? TRUE : FALSE; }

  Int32	     flags_;
  Trigger   *triggerObject_;

  //++MV, amir 
  // Used for forcing operators' cardinality
  ForceCardinalityInfo	    *forceCardinalityInfo_;
  //--MV

  // These "global" column names are used for inlining.
  static const char execIdVirtualColName_[];
  static const char epochVirtualColName_[];
  static const char mvLogTsColName_[];
  static const char rowtypeVirtualColName_[];
  static const char rowcountVirtualColName_[];

  // Binding time information needed in the trigger's transformation
  TriggerBindInfo *triggerBindInfo_;
};

//////////////////////////////////////////////////////////////////////////////
class ForceCardinalityInfo : public NABasicObject
{
public:
  // default Ctor
  ForceCardinalityInfo()
  {};
  
  // copy Ctor
  ForceCardinalityInfo(const ForceCardinalityInfo &other);

  virtual ~ForceCardinalityInfo() {}; // LCOV_EXCL_LINE

  // This value is used to force the cardinality.
  // It will be set to 0 when it should be ignored
  Cardinality cardinality_;

  // This value is used as a multiplier of the cardinality
  // It will be set to 1 when it should be ignored
  double cardinalityFactor_;
};

//////////////////////////////////////////////////////////////////////////////
// TriggerBindInfo object is used to hold the specific binding time
// information needed for the triggers' transformation (see OptTrigger.cpp)
//////////////////////////////////////////////////////////////////////////////

class TriggerBindInfo : public NABasicObject {
public:

  // default Ctor
  TriggerBindInfo(CollHeap *heap)
    : heap_(heap),
      exeId_(NULL),
      backboneIudNum_(0L),
      origIudColumnList_(NULL)
  {};
  
  // copy Ctor
  TriggerBindInfo(const TriggerBindInfo &other);

  virtual ~TriggerBindInfo() {}; // LCOV_EXCL_LINE

  // forwards definition
  typedef UniqueExecuteId *UniqueExecuteIdPtr ;

  // Accessors
  UniqueExecuteId *getExecuteId() const { return exeId_; }
  UniqueExecuteIdPtr &getExecuteId() { return exeId_; }
  Lng32 getBackboneIudNum() const { return backboneIudNum_; }
  const ColumnDescList *getIudColumnList() const { return origIudColumnList_; }

  // Mutators
  void setExecuteId() { exeId_ = new(heap_) UniqueExecuteId(); }
  void setBackboneIudNum(Lng32 iudNum) { backboneIudNum_ = iudNum; }
  void setIudColumnList(const ColumnDescList *colList) { origIudColumnList_ = colList; }

  // Normalize all the data members that need normalization
  void normalizeMembers(NormWA & normWA);

  void storeNewAndOldValues(BindWA *bindWA, const RETDesc *retDesc);

private:

  // operator= not implemented
  TriggerBindInfo &operator=(const TriggerBindInfo &other);

  CollHeap *heap_;

  // Copy of the uniquifier of the triggering action for use at triggers'
  // transformation phase (construction of the join predicate).
  // Since exeId_ is used in the tree, it is not owned by this object
  // (although it is allocated by it!), thus must not be freed in the Dtor.
  UniqueExecuteId *exeId_;
  Lng32 backboneIudNum_;

  const ColumnDescList *origIudColumnList_;
};

#endif