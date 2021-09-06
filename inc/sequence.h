
/*
Sequence 1.0.1 Sequential mission management library
This library contains implementations of sequence and basic element blocks.
Jin Kim 2021

tips:
Custom blocks can be made by inheriting Block class.
It is also possible to accommodate a sequence in a block with the SequenceBlock.
*/

#ifndef SEQUENCE_H_
#define SEQUENCE_H_

#include <iostream>
#include <vector>
#include <tuple>
#include <functional>
#include <string>
#include <cstdarg>
#include <chrono>

using namespace std;

namespace seq
{
class Block
{
private:
	virtual Block* doClone() = 0;
public:
	struct SpinInfo
	{
		std::chrono::duration<double> timeDelta;
	};
	virtual bool spinOnce(SpinInfo spinInfo) = 0;	//returns true when block is 'finished' and good to move on to the next block.
	virtual void reset() = 0;	//Sequence calls reset() and resets the block before moving on to the next one to make sure it can be used again.
	virtual void notifyStart() = 0;
	Block* clone();
};

class Sequence
{
private:
	vector<Block*> blockList;
	int currentStep;
	bool running;
	bool finished;
	bool steadyStep;
	int requestedStep;
	chrono::system_clock::time_point timeLastUpdate;

	//Recurse through variadic arguments.
	template <typename... BlockPtrs>
	void addArgs(Block* blockPtr, BlockPtrs... blockPtrs)
	{
		blockList.push_back(blockPtr);
		addArgs(blockPtrs...);
	}
	//End of recursion. Push final argument.
	void addArgs(Block* blockPtr)
	{
		blockList.push_back(blockPtr);
		return; 
	}
public:
	Sequence();
	template <typename... BlockPtrs>
	Sequence(BlockPtrs... blockPtrs):Sequence()
	{
  		addArgs(blockPtrs...);
	}

	Sequence* clone();

	~Sequence();

	void add(Block* block);
	void clear();
	bool spinOnce();	//update ongoing block or advance to the next one. Returns true when the sequence is finished.
	void step(int value);	//step blocks forcefully in sequence by value(nagative to backtrack).
	void start();	//activate the sequence.
	void suspend();	//pause the sequence without resetting the progress.
	void reset();	//stop and reset the sequence.
	bool isRunning();
	bool isFinished();	//true when the sequence is finished successfully. Its return value can be reset to false using stop() or restarting the sequence using start().
	void insertInitiator();	//insert block that waits for publisher ready.(0.1sec delay)
	void enableSteadyStep(bool value);
};

namespace block
{
class Print : public Block
{
private:
	std::string text;
	Block* doClone();
public:
	Print(const std::string& text);
	virtual void notifyStart(){}
	virtual bool spinOnce(SpinInfo spinInfo);
	virtual void reset();
	Block* clone();
};

class Delay : public Block
{
private:
	double timeSeconds;
	double timeElapsed;
	virtual Block* doClone();
public:
	Delay(int timeSeconds);
	virtual void notifyStart(){}
	virtual bool spinOnce(SpinInfo spinInfo);
	virtual void reset();
};

class Function : public Block
{
private:
	function<void(void)> func;
	virtual Block* doClone();
public:
	Function(function<void(void)> func);
	virtual void notifyStart(){}
	virtual bool spinOnce(SpinInfo spinInfo);
	virtual void reset();
};

class WaitFor : public Block
{
private:
	function<bool(void)> func;
	virtual Block* doClone();
public:
	WaitFor(function<bool(void)> func);
	virtual void notifyStart(){}
	virtual bool spinOnce(SpinInfo spinInfo);
	virtual void reset();
};

class SequenceBlock : public Block
{
private:
	virtual Block* doClone();
protected:
	Sequence* sequence;
public:
	SequenceBlock(Sequence *sequence);
	~SequenceBlock();
	virtual void notifyStart(){sequence->start();}
	virtual bool spinOnce(SpinInfo spinInfo);
	virtual void reset();
};

class LoopSequence : public SequenceBlock
{
private:
	function<bool(void)> breakCondition;
public:
	LoopSequence(Sequence *sequence, function<bool(void)> breakCondition) : SequenceBlock(sequence)
	{
		this->breakCondition = breakCondition;
	}
	virtual bool spinOnce(SpinInfo spinInfo)
	{
		if(sequence->spinOnce())
		{
			sequence->reset();
    		sequence->start();

			return breakCondition();
		}
		return false;
	}
};

}
}
#endif
