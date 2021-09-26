#include "../inc/sequence_core.h"

using namespace seq;

vector<Sequence*> Sequence::sequenceList;
Block* Sequence::ongoingBlock;

Sequence::~Sequence()
{
  clear();
}

void seq::Sequence::setHierarchyLevel(unsigned int value)
{
  hierarchyLevel = value;
}

unsigned int seq::Sequence::getHierarchyLevel()
{
  return hierarchyLevel;
}

void Sequence::add(Block *block)
{
  if (isCompiled) {
    throw InvalidOperation("Can't add blocks to already compiled sequence.");
  }
  blockList.push_back(block);
}

void Sequence::clear()
{
  //cout<<"clearing sequence"<<endl;
  for (Block *block: blockList) {
    delete (block);
  }
  blockList.clear();
  currentStep = 0;
}

bool Sequence::update(SpinInfo spinInfo)
{
  while (true) {
    //cout<<"calling "<<blockList[currentStep]->generateDebugName()<<endl;
    Sequence::ongoingBlock = blockList[currentStep];
    if (blockList[currentStep]->update(spinInfo)) {
      //step
      currentStep++;
      //reset the block, call callback
      if ((size_t) currentStep < blockList.size())
      {
        Sequence::printDebug(blockList[currentStep]->generateDebugName(), true);
        blockList[currentStep]->reset();
        blockList[currentStep]->startCallback();
      }
        //Check for end of the sequence.
      else {
        stop();
        finished = true;
        if (isOrigin)cout << string("Sequence terminated.(") + name + string(")") << endl;
        return true;
      }
    } else {
      //return when false returned.
      return false;
    }

    //exit loop.
    if (steadyStep) break;
  }

  return false;
}

bool Sequence::update()
{
  //cout<<"Running:"<<running<<" blocks:"<<currentStep<<"/"<<blockList.size()<<endl;
  if (!running) return false;

  //Calculate time duration since last spin.
  chrono::duration<double> timeUpdateDelta = chrono::system_clock::now() - timeLastUpdate;
  timeLastUpdate = chrono::system_clock::now();
  SpinInfo spinInfo;
  spinInfo.timeDelta = timeUpdateDelta.count();

  return update(spinInfo);
}

void Sequence::start()
{
  if (!blockList.empty()) {
    if (isOrigin)cout << string("Sequence started.(") + name + string(")") << endl;

    running = true;
    finished = false;
    Sequence::ongoingBlock = blockList[0];
    Sequence::printDebug(blockList[currentStep]->generateDebugName(), true);
    blockList[0]->reset();
    blockList[0]->startCallback();
  }
}

void Sequence::suspend()
{
  running = false;
}

void Sequence::stop()
{
  running = false;
  currentStep = 0;
}

bool Sequence::isRunning()
{
  return running;
}

bool Sequence::isFinished()
{
  return finished;
}

void Sequence::enableSteadyStep(bool value)
{
  steadyStep = value;
}

void Sequence::init(bool debug)
{
  if(isCompiled)
  {
    throw InvalidOperation("Sequence already compiled.");
  }

  this->debug = debug;
  for (Block *block: blockList)
  {
    block->setContainerSequence(this);
    block->init(debug);
  }
  isCompiled = true;
}

bool Sequence::debugEnabled()
{
  return debug;
}

void Sequence::print()
{
  //if(isOrigin && !isCompiled) throw InvalidOperation("init before print.");
  if(isOrigin)cout << string("Origin(") + name + string(")") << endl;
  for(Block* block : blockList)
  {
    block->print();
  }
  if(isOrigin)cout << string("Origin(") + name + string(")") << endl;
}

void Sequence::spinOnce()
{
  for(Sequence* sequence : sequenceList)
  {
    if(sequence->update())
    {

    }
  }
}

void Sequence::compile(bool debug)
{
  this->isOrigin = true;
  Sequence::sequenceList.push_back(this);
  init(debug);
}

void Sequence::printDebug(string msg, bool line)
{
  if (!ongoingBlock->getContainerSequence()->debugEnabled()) return;
  cout << "|";
  for (unsigned int i = 0; i < ongoingBlock->getContainerSequence()->getHierarchyLevel(); i++) cout << " |";
  if (line)cout << "___";
  else cout << ">  ";
  cout << msg << endl;
}

seq::Block::Block()
{
  containerSequence = nullptr;
}

string seq::Block::generateDebugName()
{
  return string("Block(") + typeid(*this).name() + string(")");
}

void Block::init(bool debug)
{

}

void Block::setContainerSequence(Sequence *sequence)
{
  this->containerSequence = sequence;
}

void Block::startCallback()
{

}
void Block::print()
{
  Sequence::printDebug(generateDebugName(), true);
}

Sequence* Block::getContainerSequence()
{
  return containerSequence;
}