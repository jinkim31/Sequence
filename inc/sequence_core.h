#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <iostream>
#include <vector>
#include <tuple>
#include <functional>
#include <string>
#include <cstdarg>
#include <chrono>
#include <stdexcept>

using namespace std;

namespace seq
{

/*Exceptions*/
class InvalidOperation : runtime_error
{
public:
    InvalidOperation(string msg) : runtime_error(msg.c_str()) {};
};

/*Structs*/
struct SpinInfo
{
    double timeDelta;
};

/*Classes*/
class Block;//forward declaration

class Sequence
{
private:
    vector<Block *> blockList;
    string name;
    bool isOrigin;
    int currentStep;
    bool running;
    bool finished;
    bool steadyStep;
    bool debug;
    unsigned int hierarchyLevel;
    bool isCompiled;
    chrono::system_clock::time_point timeLastUpdate;

    //Recurse through variadic arguments.
    template<typename... BlockPtrs>
    void addArgs(string name, BlockPtrs... blockPtrs)
    {
      this->name = name;
      addArgs(blockPtrs...);
    }

    template<typename... BlockPtrs>
    void addArgs(Block *blockPtr, BlockPtrs... blockPtrs)
    {
      add(blockPtr);
      addArgs(blockPtrs...);
    }

    //End of recursion. Push final argument.
    void addArgs(Block *blockPtr)
    {
      add(blockPtr);
      return;
    }

    static vector<Sequence*> sequenceList;
    static Block *ongoingBlock;
public:
    template<typename... BlockPtrs>
    Sequence(BlockPtrs... blockPtrs)
    {
      timeLastUpdate = chrono::system_clock::now();
      currentStep = 0;
      finished = false;
      running = false;
      steadyStep = false;
      hierarchyLevel = 0;
      isOrigin = false;
      isCompiled = false;
      name = "unnamed sequence";
      addArgs(blockPtrs...);
    }
    ~Sequence();
    void compile(bool debug = false);
    void init(bool debug); //sets hierarchy for debug
    void setHierarchyLevel(unsigned int value);
    void setAsOrigin(bool value) { isOrigin = value; }
    string geName() { return name; }
    unsigned int getHierarchyLevel();
    void add(Block *block);
    void clear();
    bool update();  //update ongoing block or advance to the next one. Returns true when the sequence is finished.
    bool update(SpinInfo spinInfo);
    void start();  //start or resume the sequence.
    void suspend();  //pause the sequence without resetting the progress.
    void stop();  //stop the sequence.
    bool isRunning();
    bool isFinished();
    void enableSteadyStep(bool value);
    bool debugEnabled();
    void print();

    static void spinOnce();
    static void printDebug(string msg, bool line=false);
};

class Block
{
protected:
    Sequence *containerSequence;
public:

    Block();

    void setContainerSequence(Sequence *sequence);
    Sequence* getContainerSequence();

    virtual string generateDebugName();

    virtual bool update(SpinInfo spinInfo) = 0;  //returns true when block is 'finished' and good to move on to the next block.
    virtual void reset() = 0;  //Sequence calls reset() before processing each block.
    virtual void startCallback(); //callback that is called once when the block is starting.
    virtual void init(bool debug);
    virtual void print();
};
}
#endif