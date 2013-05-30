/*
 * Temporary class to get i-score working with Score and Modular
 * Copyright © 2013, Théo de la Hogue
 *
 * License: This code is licensed under the terms of the "CeCILL-C"
 * http://www.cecill.info
 */

#include "Engine.h"

#include <stdio.h>
#include <math.h>

using namespace std;

/*!
 * \file Engine.cpp
 * \author Théo de la Hogue, based on Engine.cpp written by Raphael Marczak (LaBRI) for the libIscore.
 * \date 2012-2013
 */

EngineCacheElement::EngineCacheElement()
{
    ;
}

EngineCacheElement::~EngineCacheElement()
{
    ;
}

Engine::Engine(void(*interactiveEventActiveAttributeCallback)(InteractiveProcessId, bool),
               void(*timeProcessSchedulerRunningAttributeCallback)(TimeProcessId, bool),
               void(*transportDataValueCallback)(TTSymbol&, const TTValue&))
{
    m_InteractiveEventActiveAttributeCallback = interactiveEventActiveAttributeCallback;
    m_TimeProcessSchedulerRunningAttributeCallback = timeProcessSchedulerRunningAttributeCallback;
    m_TransportDataValueCallback = transportDataValueCallback;
    
    m_nextTimeProcessId = 1;
    m_nextIntervalId = 1;
    m_nextInteractiveProcessId = 1;
    
    initModular();
    initScore();
}

void Engine::initModular()
{
    TTErr           err;
    TTValue         args, v;
    TTString        applicationName = "i-score";                  // TODO : declare as global variable
    TTObjectBasePtr iscore;                                       // TODO : declare as global variable
    TTString        configFile = "/usr/local/include/IScore/i-scoreConfiguration.xml";
    TTHashPtr       hashParameters;
    
    // this initializes the Modular framework and loads protocol plugins (in /usr/local/jamoma/extensions folder)
    TTModularInit();
    
    // create a local application named i-score
    TTModularCreateLocalApplication(applicationName, configFile);
    
    // get i-score application
    iscore = getLocalApplication;
    
    // create TTData for tranport service and expose them
    
    // Play
    TTValuePtr batonPlay = new TTValue(TTPtr(this));
    batonPlay->append(TTSymbol("Play"));
    TTModularCreateData(kTTSym_message, batonPlay, &TransportDataValueCallback, &m_dataPlay);
    
    m_dataPlay->setAttributeValue(kTTSym_type, kTTSym_none);
    m_dataPlay->setAttributeValue(kTTSym_description, TTSymbol("start i-score execution"));
    
    TTModularRegisterObject(TTAddress("/Transport/Play"), m_dataPlay);
    
    // Stop
    TTValuePtr batonStop = new TTValue(TTPtr(this));
    batonStop->append(TTSymbol("Stop"));
    TTModularCreateData(kTTSym_message, batonStop, &TransportDataValueCallback, &m_dataStop);
    
    m_dataStop->setAttributeValue(kTTSym_type, kTTSym_none);
    m_dataStop->setAttributeValue(kTTSym_description, TTSymbol("stop i-score execution"));
    
    TTModularRegisterObject(TTAddress("/Transport/Stop"), m_dataStop);
    
    // Pause
    TTValuePtr batonPause = new TTValue(TTPtr(this));
    batonPause->append(TTSymbol("Pause"));
    TTModularCreateData(kTTSym_message, batonPause, &TransportDataValueCallback, &m_dataPause);
    
    m_dataPause->setAttributeValue(kTTSym_type, kTTSym_none);
    m_dataPause->setAttributeValue(kTTSym_description, TTSymbol("pause i-score execution"));
    
    TTModularRegisterObject(TTAddress("/Transport/Pause"), m_dataPause);
    
    // Rewind
    TTValuePtr batonRewind = new TTValue(TTPtr(this));
    batonRewind->append(TTSymbol("Rewind"));
    TTModularCreateData(kTTSym_message, batonRewind, &TransportDataValueCallback, &m_dataRewind);
    
    m_dataRewind->setAttributeValue(kTTSym_type, kTTSym_none);
    m_dataRewind->setAttributeValue(kTTSym_description, TTSymbol("return to the beginning of the scenario"));
    
    TTModularRegisterObject(TTAddress("/Transport/Rewind"), m_dataRewind);
    
    // StartPoint
    TTValuePtr batonStartPoint = new TTValue(TTPtr(this));
    batonStartPoint->append(TTSymbol("StartPoint"));
    TTModularCreateData(kTTSym_message, batonStartPoint, &TransportDataValueCallback, &m_dataStartPoint);
    
    m_dataStartPoint->setAttributeValue(kTTSym_type, kTTSym_integer);
    m_dataStartPoint->setAttributeValue(kTTSym_description, TTSymbol("set the time where to start i-score execution (in ms)"));
    
    TTModularRegisterObject(TTAddress("/Transport/StartPoint"), m_dataStartPoint);
    
    // Speed
    TTValuePtr batonSpeed = new TTValue(TTPtr(this));
    batonSpeed->append(TTSymbol("Speed"));
    TTModularCreateData(kTTSym_message, batonSpeed, &TransportDataValueCallback, &m_dataSpeed);
    
    m_dataSpeed->setAttributeValue(kTTSym_type, kTTSym_decimal);
    v = TTValue(0., 10.);
    m_dataSpeed->setAttributeValue(kTTSym_rangeBounds, v);
    m_dataSpeed->setAttributeValue(kTTSym_description, TTSymbol("change i-score speed rate execution"));
    
    TTModularRegisterObject(TTAddress("/Transport/Speed"), m_dataSpeed);
    
    
    ////////////
    // Example : Create a distant application
    ////////////
    TTObjectBasePtr anApplication = NULL;
    TTSymbol        MinuitApplicationName;
    
    // create an application called MinuitDevice1
    MinuitApplicationName = TTSymbol("MinuitDevice1");
    args = TTValue(MinuitApplicationName);
    TTObjectBaseInstantiate(kTTSym_Application, TTObjectBaseHandle(&anApplication), args);
    
    // set application type : here 'mirror' because it use Minuit protocol
    anApplication->setAttributeValue(kTTSym_type, TTSymbol("mirror"));
    
    ////////////
    // Example : Register i-score and MinuitDevice1 to the Minuit protocol
    ////////////
    
    // check if the Minuit protocol has been loaded
    if (getProtocol(TTSymbol("Minuit"))) {
        
        // register the local application to the Minuit protocol
        getProtocol(TTSymbol("Minuit"))->sendMessage(TTSymbol("registerApplication"), TTSymbol(applicationName), kTTValNONE);
        
        // get parameter's table
        v = TTSymbol(applicationName);
        err = getProtocol(TTSymbol("Minuit"))->getAttributeValue(TTSymbol("applicationParameters"), v);
        
        if (!err) {
            
            hashParameters = TTHashPtr((TTPtr)v[0]);
        
            // replace the Minuit parameters for the local application
            hashParameters->remove(TTSymbol("port"));
            hashParameters->append(TTSymbol("port"), 8002);
            
            hashParameters->remove(TTSymbol("ip"));
            hashParameters->append(TTSymbol("ip"), TTSymbol("127.0.0.1"));
        
            v = TTSymbol(applicationName);
            v.append((TTPtr)hashParameters);
            getProtocol(TTSymbol("Minuit"))->setAttributeValue(TTSymbol("applicationParameters"), v);
        }
        
        // register this application to the Minuit protocol
        getProtocol(TTSymbol("Minuit"))->sendMessage(TTSymbol("registerApplication"), MinuitApplicationName, kTTValNONE);
        
        // get parameter's table
        v = TTSymbol(MinuitApplicationName);
        err = getProtocol(TTSymbol("Minuit"))->getAttributeValue(TTSymbol("applicationParameters"), v);
        
        if (!err) {
            
            hashParameters = TTHashPtr((TTPtr)v[0]);
        
            // replace the Minuit parameters for the distant application
            hashParameters->remove(TTSymbol("port"));
            hashParameters->append(TTSymbol("port"), 9998);
            
            hashParameters->remove(TTSymbol("ip"));
            hashParameters->append(TTSymbol("ip"), TTSymbol("127.0.0.1"));
        
            v = TTValue(MinuitApplicationName);
            v.append((TTPtr)hashParameters);
            getProtocol(TTSymbol("Minuit"))->setAttributeValue(TTSymbol("applicationParameters"), v);
        }
        
        // run the Minuit protocol
        TTModularApplications->sendMessage(TTSymbol("ProtocolRun"), TTSymbol("Minuit"), kTTValNONE);
        
        ////////////
        // Example : Build the namespace of MinuitDevice1 using discovery feature (if the protocol provides it)
        ////////////
        
        // you can create an OSC receive on the 9998 port to see that a namespace request is sent by i-score (using Pure Data for example)
        anApplication->sendMessage(TTSymbol("DirectoryBuild"));
    }
/*
    ////////////
    // Example : Read the namespace of MinuitDevice1 from a namespace file
    ////////////
    
    // create a TTXmlHandler class to parse a xml namespace file
    TTXmlHandlerPtr myXmlHandler = NULL;
    TTObjectBaseInstantiate(kTTSym_XmlHandler, TTObjectBaseHandle(&myXmlHandler), kTTValNONE);
    
    // prepare the TTXmlHandler to pass the result of the parsing to MinuitDevice1
    v = TTValue(anApplication);
    myXmlHandler->setAttributeValue(TTSymbol("object"), v);
    
    // read a namespace file
    err = myXmlHandler->sendMessage(TTSymbol("Read"), TTSymbol("/Users/WALL-E/Documents/Jamoma/Modules/Modular/implementations/MaxMSP/jcom.modular/remoteApp - namespace.xml"), kTTValNONE);
    
    if (!err) {
        // get the root of the TNodeDirectory of MinuitDevice1
        // note : the TTNodeDirectory is a tree structure used to registered and retrieve TTObjects using TTAddress
        TTNodeDirectoryPtr anApplicationDirectory = getApplicationDirectory(TTSymbol("MinuitDevice1"));
        
        // return all addresses of the TTNodeDirectory
        std::cout << "__ Dump MinuitDevice1 directory __" << std::endl;
        
        // start to dump addresses recursilvely from the root of the directory
        dumpAddressBelow(anApplicationDirectory->getRoot());
        
        std::cout << "__________________________________" << std::endl;
    }
*/    
}

void Engine::initScore()
{
    TTValue args, v;
    
    // this initializes the Score framework
    TTScoreInit();
    
    // Create the main scenario
    m_mainScenario = NULL;
    TTObjectBaseInstantiate(TTSymbol("Scenario"), TTObjectBaseHandle(&m_mainScenario), args);
    
    // Create a static time event for the start and set his date in the mean time
    v = TTValue(TTSymbol("StaticEvent"));
    v.append(TTUInt32(0));
    m_mainScenario->sendMessage(TTSymbol("StartEventCreate"), v, kTTValNONE);
    
    // Create a static time event for the end and set his date in the mean time
    v = TTValue(TTSymbol("StaticEvent"));
    v.append(TTUInt32(SCENARIO_SIZE));
    m_mainScenario->sendMessage(TTSymbol("EndEventCreate"), v, kTTValNONE);
    
    // TODO : create a TTCallback to observe each time process scheduler running attribute (using TimeProcessSchedulerRunningAttributeCallback)
    
    // Store the main scenario (so ROOT_BOX_ID is 1)
    cacheTimeProcess(m_mainScenario);
}

void Engine::dumpAddressBelow(TTNodePtr aNode)
{
    TTList returnedChildren;
    TTAddress anAddress;
    
    // fill a TTList with all children (because we use * (wilcard) for the name and the instance)
    aNode->getChildren(S_WILDCARD, S_WILDCARD, returnedChildren);
    
    // for each child
    for (returnedChildren.begin(); returnedChildren.end(); returnedChildren.next()) {
        // get a node from the TTList
        aNode = TTNodePtr((TTPtr)returnedChildren.current()[0]);
        
        // compute his absolute address
        aNode->getAddress(anAddress);
        
        std::cout << "   " << anAddress.string() << std::endl;
        
        // dump all addresses below this node (recursive call)
        dumpAddressBelow(aNode);
    }
}

Engine::~Engine()
{
    TTObjectBaseRelease(TTObjectBaseHandle(&m_mainScenario));
}

TimeProcessId Engine::cacheTimeProcess(TimeProcessPtr timeProcess)
{
    TimeProcessId id;
    EngineCacheElementPtr e;
    
    e = new EngineCacheElement();
    e->object = TTObjectBasePtr(timeProcess);
    
    id = m_nextTimeProcessId;
    m_timeProcessMap[id] = e;
    m_nextTimeProcessId++;
    
    return id;
}

TimeProcessPtr Engine::getTimeProcess(TimeProcessId boxId)
{
    return TimeProcessPtr(TTObjectBasePtr(m_timeProcessMap[boxId]->object));
}

void Engine::uncacheTimeProcess(TimeProcessId boxId)
{
    EngineCacheElementPtr e = m_timeProcessMap[boxId];
    
    delete e;
    m_timeProcessMap.erase(boxId);
}

IntervalId Engine::cacheInterval(TimeProcessPtr timeProcess)
{
    TimeProcessId id;
    EngineCacheElementPtr e;
    
    e = new EngineCacheElement();
    e->object = TTObjectBasePtr(timeProcess);
    
    id = m_nextIntervalId;
    m_intervalMap[id] = e;
    m_nextIntervalId++;
    
    return id;
}

TimeProcessPtr Engine::getInterval(IntervalId relationId)
{
    return TimeProcessPtr(TTObjectBasePtr(m_intervalMap[relationId]->object));
}

void Engine::uncacheInterval(IntervalId relationId)
{
    EngineCacheElementPtr e = m_intervalMap[relationId];
    
    delete e;
    m_intervalMap.erase(relationId);
}

InteractiveProcessId Engine::cacheInteractiveProcess(TimeProcessPtr timeProcess, TimeEventIndex controlPointId)
{
    InteractiveProcessId id;
    EngineCacheElementPtr e;
    
    e = new EngineCacheElement();
    e->object = timeProcess;
    e->index = controlPointId;
    
    id = m_nextInteractiveProcessId;
    m_interactiveProcessMap[id] = e;
    m_nextInteractiveProcessId++;
    
    return id;
}

TimeProcessPtr Engine::getInteractiveProcess(InteractiveProcessId triggerId, TimeEventIndex& controlPointId)
{
    controlPointId = m_interactiveProcessMap[triggerId]->index;
    
    return TimeProcessPtr(TTObjectBasePtr(m_interactiveProcessMap[triggerId]->object));
}

void Engine::uncacheInteractiveProcess(InteractiveProcessId triggerId)
{
    EngineCacheElementPtr e = m_interactiveProcessMap[triggerId];
    
    delete e;
    m_interactiveProcessMap.erase(triggerId);
}

// Edition ////////////////////////////////////////////////////////////////////////

TimeProcessId Engine::addBox(TimeValue boxBeginPos, TimeValue boxLength, TimeProcessId motherId)
{
    TimeProcessPtr  parentScenario = getTimeProcess(motherId);
    TimeProcessPtr  timeProcess;
    TimeProcessId   boxId;
    TTValue         v;
    
    // Create a new automation time process
    timeProcess = NULL;
    TTObjectBaseInstantiate(TTSymbol("Automation"), TTObjectBaseHandle(&timeProcess), kTTValNONE);
    
    // Create a static time event for the start and set his date in the mean time
    v = TTValue(TTSymbol("StaticEvent"));
    v.append(TTUInt32(boxBeginPos));
    timeProcess->sendMessage(TTSymbol("StartEventCreate"), v, kTTValNONE);
    
    // Create a static time event for the end and set his date in the mean time
    v = TTValue(TTSymbol("StaticEvent"));
    v.append(TTUInt32(boxBeginPos+boxLength));
    timeProcess->sendMessage(TTSymbol("EndEventCreate"), v, kTTValNONE);
    
    // Add the time process to the parent scenario 
    v = TTObjectBasePtr(parentScenario);
    timeProcess->setAttributeValue(TTSymbol("scenario"), v);
    
    // Cache it and get an unique id for this process
    boxId = cacheTimeProcess(timeProcess);
    
    // TODO : create a TTCallback to observe each time process event active attribute (using InteractiveEventActiveAttributeCallback)
    // getTimeProcess(boxId)->appendObserver() ?
    
    // TODO : create a TTCallback to observe each time process scheduler running attribute (using TimeProcessSchedulerRunningAttributeCallback)
    // getTimeProcess(boxId)->appendObserver() ?
    
	return boxId;
}

void Engine::removeBox(TimeProcessId boxId)
{
    TimeProcessPtr          timeProcess;
    EngineCacheMapIterator  it;
    InteractiveProcessId    id1 = 0;
    InteractiveProcessId    id2 = 0;
    TTValue                 v;
    
    // Retreive the time process using the boxId
    timeProcess = getTimeProcess(boxId);
    
    // Remove the time process from the scenario
    timeProcess->setAttributeValue(TTSymbol("scenario"), kTTValNONE);
    
    // TODO : delete TTCallback used to observe each time process scheduler running attribute
    // getTimeProcess(boxId)->removeObserver() ?
    
    // Release start and end events
    timeProcess->sendMessage(TTSymbol("StartEventRelease"));
    timeProcess->sendMessage(TTSymbol("EndEventRelease"));
    
    // Is it an interactiveProcess ?
    // note : it can be registered 2 times for a start and end interactive event
    for (it = m_interactiveProcessMap.begin(); it != m_interactiveProcessMap.end(); ++it) {
        
        if (it->second->object == timeProcess) {
         
            if (!id1) id1 = it->first;
            else if (!id2) id2 = it->first;
        }
    }
    
    if (id1) uncacheInteractiveProcess(id1);
    if (id2) uncacheInteractiveProcess(id2);
    
    // Delete the time process
    TTObjectBaseRelease(TTObjectBaseHandle(&timeProcess));
    
    // Remove the time process from the cache
    uncacheTimeProcess(boxId);
}

IntervalId Engine::addTemporalRelation(TimeProcessId boxId1,
                                       TimeEventIndex controlPoint1,
                                       TimeProcessId boxId2,
                                       TimeEventIndex controlPoint2,
                                       TemporalRelationType type,
                                       vector<TimeProcessId>& movedBoxes)
{
    TimeProcessPtr          timeProcess, tp1, tp2;
    IntervalId              relationId;
    TTValue                 v;
    EngineCacheMapIterator  it;
    TTErr                   err;
    
    // Create a new interval time process
    timeProcess = NULL;
    TTObjectBaseInstantiate(TTSymbol("Interval"), TTObjectBaseHandle(&timeProcess), kTTValNONE);
    
    // Cache it and get an unique id for this process
    relationId = cacheInterval(timeProcess);
    
    // Get the events from the given box ids and pass them to the time process
    tp1 = getTimeProcess(boxId1);
    tp2 = getTimeProcess(boxId2);
    
    if (controlPoint1 == BEGIN_CONTROL_POINT_INDEX)
        tp1->getAttributeValue(TTSymbol("startEvent"), v);
    else
        tp1->getAttributeValue(TTSymbol("endEvent"), v);
    
    timeProcess->setAttributeValue(TTSymbol("startEvent"), v);
    
    if (controlPoint2 == BEGIN_CONTROL_POINT_INDEX)
        tp2->getAttributeValue(TTSymbol("startEvent"), v);
    else
        tp2->getAttributeValue(TTSymbol("endEvent"), v);
    
    timeProcess->setAttributeValue(TTSymbol("endEvent"), v);
    
    // TODO : order the events
    
    // Set the time process rigid
    v = TTBoolean(YES);
    timeProcess->setAttributeValue(TTSymbol("rigid"), v);
    
    // Add the time process to the main scenario
    v = TTObjectBasePtr(m_mainScenario);
    err = timeProcess->setAttributeValue(TTSymbol("scenario"), v);
    
    // an error can append if the start is after the end
    if (!err) {
    
        // return the entire timeProcessMap except the first process !!! (this is bad but it is like former engine)
        it = m_timeProcessMap.begin();
        it++;
        for (; it != m_timeProcessMap.end(); ++it)
            movedBoxes.push_back(it->first);
    
        return relationId;
    }
    else {
        
        // Remove the interval from the cache
        removeTemporalRelation(relationId);
        
        return NO_ID;
    }
}

void Engine::removeTemporalRelation(IntervalId relationId)
{
    TimeProcessPtr  timeProcess;
    
    // Retreive the interval using the relationId
    timeProcess = getInterval(relationId);
    
    // Remove the time process from the scenario
    timeProcess->setAttributeValue(TTSymbol("scenario"), kTTValNONE);
    
    // Delete the interval
    TTObjectBaseRelease(TTObjectBaseHandle(&timeProcess));
    
    // Remove the interval from the cache
    uncacheInterval(relationId);
}

void Engine::changeTemporalRelationBounds(IntervalId relationId, BoundValue minBound, BoundValue maxBound, vector<TimeProcessId>& movedBoxes)
{
    TimeProcessPtr          timeProcess = getInterval(relationId);
    EngineCacheMapIterator  it;
    TTValue                 v;
    
    // filtering NO_BOUND (-1) and negative value because we use unsigned int
    if (minBound == NO_BOUND)
        v = TTValue(TTUInt32(0));
    else if (minBound < 0)
        v = TTValue(TTUInt32(abs(minBound)));
    else
        v = TTValue(TTUInt32(minBound));
    
    if (maxBound == NO_BOUND)
        v.append(TTUInt32(0));
    else if (maxBound < 0)
        v = TTValue(TTUInt32(abs(maxBound)));
    else
        v.append(TTUInt32(maxBound));
    
    // NOTE : sending 0 0 means the relation is not rigid
    // NOTE : it is also possible to use the "rigid" attribute to swicth between those two states
    timeProcess->sendMessage(TTSymbol("Limit"), v, kTTValNONE);
    
    // return the entire timeProcessMap except the first process !!! (this is bad but it is like former engine)
    it = m_timeProcessMap.begin();
    it++;
    for (; it != m_timeProcessMap.end(); ++it)
        movedBoxes.push_back(it->first);
}

bool Engine::isTemporalRelationExisting(TimeProcessId boxId1, TimeEventIndex controlPoint1, TimeProcessId boxId2, TimeEventIndex controlPoint2)
{
    TTValue                 v1, v2, v;
    TimeProcessPtr          timeProcess, tp1, tp2;
    IntervalId              found = NO_ID;
    EngineCacheMapIterator  it;
    
    // Get the events from the given box ids and pass them to the time process
    tp1 = getTimeProcess(boxId1);
    tp2 = getTimeProcess(boxId2);
    
    if (controlPoint1 == BEGIN_CONTROL_POINT_INDEX)
        tp1->getAttributeValue(TTSymbol("startEvent"), v1);
    else
        tp1->getAttributeValue(TTSymbol("endEvent"), v1);
    
    if (controlPoint2 == BEGIN_CONTROL_POINT_INDEX)
        tp2->getAttributeValue(TTSymbol("startEvent"), v2);
    else
        tp2->getAttributeValue(TTSymbol("endEvent"), v2);
    
    // Look into the interval map to retreive an interval with the same events
    for (it = m_intervalMap.begin(); it != m_intervalMap.end(); ++it) {
        
        timeProcess = TimeProcessPtr(TTObjectBasePtr(it->second->object));
        
        timeProcess->getAttributeValue(TTSymbol("startEvent"), v);
        
        if (v == v1) {
            
            timeProcess->getAttributeValue(TTSymbol("endEvent"), v);
            
            if (v == v2) {
                found = it->first;
                break;
            }
        }
    }
    
	return found != NO_ID;
}

TimeProcessId Engine::getRelationFirstBoxId(IntervalId relationId)
{
    TTValue                 v, vt;
	TimeProcessPtr          timeProcess = getInterval(relationId);
    TimeEventPtr            event;
    TimeProcessId           found = NO_ID;
    EngineCacheMapIterator  it;
    
    // get the start event of the interval
	timeProcess->getAttributeValue(TTSymbol("startEvent"), v);
    event = TimeEventPtr(TTObjectBasePtr(v[0]));
    
    // Look into the automation map to retreive an automation with the same event
    for (it = m_timeProcessMap.begin(); it != m_timeProcessMap.end(); ++it) {
        
        timeProcess = TimeProcessPtr(TTObjectBasePtr(it->second->object));
        
        timeProcess->getAttributeValue(TTSymbol("startEvent"), vt);
        
        if (v == vt) {
            found = it->first;
            break;
        }
        
        timeProcess->getAttributeValue(TTSymbol("endEvent"), vt);
        
        if (v == vt) {
            found = it->first;
            break;
        }
    }
    
    return found;
}

TimeEventIndex Engine::getRelationFirstCtrlPointIndex(IntervalId relationId)
{
	return BEGIN_CONTROL_POINT_INDEX;
}

TimeProcessId Engine::getRelationSecondBoxId(IntervalId relationId)
{
    TTValue                 v, vt;
	TimeProcessPtr          timeProcess = getInterval(relationId);
    TimeEventPtr            event;
    TimeProcessId           found = NO_ID;
    EngineCacheMapIterator  it;
    
    // Get the end event of the interval
	timeProcess->getAttributeValue(TTSymbol("endEvent"), v);
    event = TimeEventPtr(TTObjectBasePtr(v[0]));
    
    // Look into the automation map to retreive an automation with the same event
    for (it = m_timeProcessMap.begin(); it != m_timeProcessMap.end(); ++it) {
        
        timeProcess = TimeProcessPtr(TTObjectBasePtr(it->second->object));
        
        timeProcess->getAttributeValue(TTSymbol("startEvent"), vt);
        
        if (v == vt) {
            found = it->first;
            break;
        }
        
        timeProcess->getAttributeValue(TTSymbol("endEvent"), vt);
        
        if (v == vt) {
            found = it->first;
            break;
        }
    }
    
    return found;
}

TimeEventIndex Engine::getRelationSecondCtrlPointIndex(IntervalId relationId)
{
	return END_CONTROL_POINT_INDEX;
}

BoundValue Engine::getRelationMinBound(IntervalId relationId)
{
	TimeProcessPtr  timeProcess = getInterval(relationId);
    TTValue         v;
    
    timeProcess->getAttributeValue(TTSymbol("durationMin"), v);
    
    return v[0];
}

BoundValue Engine::getRelationMaxBound(IntervalId relationId)
{
    TimeProcessPtr  timeProcess = getInterval(relationId);
    TTValue         v;

    timeProcess->getAttributeValue(TTSymbol("durationMax"), v);
    
    return v[0];
}

bool Engine::performBoxEditing(TimeProcessId boxId, TimeValue start, TimeValue end, vector<TimeProcessId>& movedBoxes)
{
    TimeProcessPtr          timeProcess = getTimeProcess(boxId);
    TTValue                 v;
    EngineCacheMapIterator  it;
    TTErr                   err;
    
    v = TTValue(start);
    v.append(end);
    
    err = timeProcess->sendMessage(TTSymbol("Move"), v, kTTValNONE);
    
    // return the entire timeProcessMap except the first process !!! (this is bad but it is like former engine)
    it = m_timeProcessMap.begin();
    it++;
    for (; it != m_timeProcessMap.end(); ++it)
        movedBoxes.push_back(it->first);
    
    return !err;
}

TimeValue Engine::getBoxBeginTime(TimeProcessId boxId)
{
	TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TTValue         v;
    
	timeProcess->getAttributeValue(TTSymbol("startDate"), v);
    
    return v[0];
}

TimeValue Engine::getBoxEndTime(TimeProcessId boxId)
{
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TTValue         v;
    
	timeProcess->getAttributeValue(TTSymbol("endDate"), v);
    
    return v[0];
}

TimeValue Engine::getBoxDuration(TimeProcessId boxId)
{
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TTValue         v;
    
	timeProcess->getAttributeValue(TTSymbol("duration"), v);
    
    return v[0];
}

int Engine::getBoxNbCtrlPoints(TimeProcessId boxId)
{
	TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TTValue         v;
    
	timeProcess->getAttributeValue(TTSymbol("intermediateEvents"), v);
    
    return v.size() + 2; // because there is always a start and an end event too
}

TimeEventIndex Engine::getBoxFirstCtrlPointIndex(TimeProcessId boxId)
{
	return BEGIN_CONTROL_POINT_INDEX;
}

TimeEventIndex Engine::getBoxLastCtrlPointIndex(TimeProcessId boxId)
{
	return END_CONTROL_POINT_INDEX;
}

void Engine::setCtrlPointMessagesToSend(TimeProcessId boxId, TimeEventIndex controlPointIndex, std::vector<std::string> messageToSend, bool muteState)
{
    TTValue         v;
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TimeEventPtr    event;
    TTObjectBasePtr state;
    TTUInt32        i;

    // Get the start or end event
    if (controlPointIndex == BEGIN_CONTROL_POINT_INDEX)
        timeProcess->getAttributeValue(TTSymbol("startEvent"), v);
    else
        timeProcess->getAttributeValue(TTSymbol("endEvent"), v);
    
    event = TimeEventPtr(TTObjectBasePtr(v[0]));
    
    // get the state of the event
    event->getAttributeValue(TTSymbol("state"), v);
    state = v[0];
    
    // clear the state
    state->sendMessage(TTSymbol("Clear"));
    
    // parse each incoming string into < directory:/address, value >
    for (i = 0; i < messageToSend.size(); i++) {
        
        TTValue v = TTString(messageToSend[i]);
        v.fromString();
        
        TTSymbol aSymbol = v[0];
		TTAddress anAddress = toTTAddress(aSymbol.string().data());
        v[0] = anAddress;
        
        // append a line to the state
        state->sendMessage(TTSymbol("Append"), v, kTTValNONE);
    }
    
    // Flatten the state to increase the recall
    state->sendMessage(TTSymbol("Flatten"));
}

void Engine::getCtrlPointMessagesToSend(TimeProcessId boxId, TimeEventIndex controlPointIndex, std::vector<std::string>& messages)
{
    TTValue         v;
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TimeEventPtr    event;
    TTObjectBasePtr state;
    TTListPtr       lines = NULL;
    TTDictionaryPtr aLine;
    TTAddress       address;
    std::string     s;
    
    // Get the start or end event
    if (controlPointIndex == BEGIN_CONTROL_POINT_INDEX)
        timeProcess->getAttributeValue(TTSymbol("startEvent"), v);
    else
        timeProcess->getAttributeValue(TTSymbol("endEvent"), v);
    
    event = TimeEventPtr(TTObjectBasePtr(v[0]));
    
    // get the state of the event
    event->getAttributeValue(TTSymbol("state"), v);
    state = v[0];
    
    // get the state lines
    state->getAttributeValue(TTSymbol("flattenedLines"), v);
    lines = TTListPtr((TTPtr)v[0]);
    
    if (lines) {
        
        // edit each line address into a "directory/address value" string
        for (lines->begin(); lines->end(); lines->next()) {
            
            aLine = TTDictionaryPtr((TTPtr)lines->current()[0]);
            
            // get the target address
            aLine->lookup(kTTSym_target, v);
            address = v[0];
            
            // get value
            aLine->getValue(v);
            v.toString();
            
            // edit string
            s = toNetworkTreeAddress(address);
            s += " ";
            s += TTString(v[0]).c_str();
            
            messages.push_back(s);
        }
    }
}

void Engine::setCtrlPointMutingState(TimeProcessId boxId, TimeEventIndex controlPointIndex, bool mute)
{
#ifdef TODO_ENGINE
	ECOProcess* currentProcess = m_executionMachine->getProcess(boxId);
    
	if ((currentProcess->getType() == PROCESS_TYPE_NETWORK_MESSAGE_TO_SEND)) {
		SendNetworkMessageProcess* currentSendOSCProcess = (SendNetworkMessageProcess*) currentProcess;
		ControlPoint* currentControlPoint = m_editor->getBoxById(boxId)->getControlPoint(controlPointIndex);
		currentSendOSCProcess->setMessageMuteState(currentControlPoint->getProcessStepId(), mute);
	}
#endif
}

//CURVES ////////////////////////////////////////////////////////////////////////////////////

void Engine::addCurve(TimeProcessId boxId, const std::string & address)
{
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    
    // remove the curve addresses of the automation time process
    timeProcess->sendMessage(TTSymbol("CurveAdd"), toTTAddress(address), kTTValNONE);
}

void Engine::removeCurve(TimeProcessId boxId, const std::string & address)
{
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    
    // remove the curve addresses of the automation time process
    timeProcess->sendMessage(TTSymbol("CurveRemove"), toTTAddress(address), kTTValNONE);
}

void Engine::clearCurves(TimeProcessId boxId)
{
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    
    // clear all the curves of the automation time process
    timeProcess->sendMessage(TTSymbol("Clear"));
}

std::vector<std::string> Engine::getCurvesAddress(TimeProcessId boxId)
{
    std::vector<std::string> curveAddresses;
    
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TTValue         v;
    
    // get the curve addresses of the automation time process
    timeProcess->getAttributeValue(TTSymbol("curveAddresses"), v);
    
    // copy the addresses into the vector
    for (TTUInt32 i = 0; i < v.size(); i++)
        curveAddresses.push_back(toNetworkTreeAddress(v[i]));
    
	return curveAddresses;
}

void Engine::setCurveSampleRate(TimeProcessId boxId, const std::string & address, unsigned int nbSamplesBySec)
{
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TTObjectBasePtr curve;
    TTValue         v;
    TTErr           err;
    
    // get curve object at address
    err = timeProcess->sendMessage(TTSymbol("CurveGet"), toTTAddress(address), v);
    
    if (!err) {
        
        curve = v[0];
        
        curve->setAttributeValue(TTSymbol("sampleRate"), TTUInt32(nbSamplesBySec));
    }
}

unsigned int Engine::getCurveSampleRate(TimeProcessId boxId, const std::string & address)
{
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TTObjectBasePtr curve;
    TTValue         v;
    TTErr           err;
    
    // get curve object at address
    err = timeProcess->sendMessage(TTSymbol("CurveGet"), toTTAddress(address), v);
    
    if (!err) {
        
        curve = v[0];
        
        curve->getAttributeValue(TTSymbol("sampleRate"), v);
        
        return TTUInt32(v[0]);
    }
    
	return 0;
}

void Engine::setCurveRedundancy(TimeProcessId boxId, const std::string & address, bool redundancy)
{
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TTObjectBasePtr curve;
    TTValue         v;
    TTErr           err;
    
    // get curve object at address
    err = timeProcess->sendMessage(TTSymbol("CurveGet"), toTTAddress(address), v);
    
    if (!err) {
        
        curve = v[0];
        
        curve->setAttributeValue(TTSymbol("redundancy"), redundancy);
    }
}

bool Engine::getCurveRedundancy(TimeProcessId boxId, const std::string & address)
{
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TTObjectBasePtr curve;
    TTValue         v;
    TTErr           err;
    
    // get curve object at address
    err = timeProcess->sendMessage(TTSymbol("CurveGet"), toTTAddress(address), v);
    
    if (!err) {
        
        curve = v[0];
        
        curve->getAttributeValue(TTSymbol("redundancy"), v);
        
        return TTBoolean(v[0]);
    }
    
	return false;
}

void Engine::setCurveMuteState(TimeProcessId boxId, const std::string & address, bool muteState)
{
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TTObjectBasePtr curve;
    TTValue         v;
    TTErr           err;
    
    // get curve object at address
    err = timeProcess->sendMessage(TTSymbol("CurveGet"), toTTAddress(address), v);
    
    if (!err) {
        
        curve = v[0];
        
        curve->setAttributeValue(TTSymbol("active"), !muteState);
    }
}

bool Engine::getCurveMuteState(TimeProcessId boxId, const std::string & address)
{
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TTObjectBasePtr curve;
    TTValue         v;
    TTErr           err;
    
    // get curve object at address
    err = timeProcess->sendMessage(TTSymbol("CurveGet"), toTTAddress(address), v);
    
    if (!err) {
        
        curve = v[0];
        
        curve->getAttributeValue(TTSymbol("active"), v);
        
        return !TTBoolean(v[0]);
    }
    
	return false;
}

void Engine::getCurveArgTypes(std::string stringToParse, std::vector<std::string>& result)
{
    TTSymbol    address;
    TTValue     v = TTString(stringToParse);
    TTUInt32    i;
    
    v.fromString();
    address = v[0];
    
    result.clear();
    result.push_back(address.string().c_str());
    
    for (i = 1; i < v.size(); i++) {
		
		if (v[i].type() == kTypeFloat32 || v[i].type() == kTypeFloat64)
			result.push_back("FLOAT");
		else if (v[i].type() == kTypeInt32 || v[i].type() == kTypeInt64)
			result.push_back("INT");
		else
			result.push_back("SYMBOL");
	}
}

bool Engine::setCurveSections(TimeProcessId boxId, std::string address, unsigned int argNb, const std::vector<float> & percent, const std::vector<float> & y, const std::vector<short> & sectionType, const std::vector<float> & coeff)
{
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TTObjectBasePtr curve;
    TTValue         v;
    TTUInt32        i, nbPoints = percent.size();
    TTErr           err;
    
    // get curve object at address
    err = timeProcess->sendMessage(TTSymbol("CurveGet"), toTTAddress(address), v);
    
    if (!err) {
        
        curve = v[0];
        
        // edit value as : x1 y1 b1 x2 y2 b2
        v.resize(nbPoints * 3);
        
        for (i = 0; i < v.size(); i = i+3) {
            
            v[i] = TTFloat64(percent[i/3] / 100.);
            v[i+1] = TTFloat64(y[i/3]);
            v[i+2] = TTFloat64(coeff[i/3]);
        }
        
        // set a curve parameters
        err = curve->setAttributeValue(TTSymbol("parameters"), v);
    }
    
    return err == kTTErrNone;
}

bool Engine::getCurveSections(TimeProcessId boxId, std::string address, unsigned int argNb,
                              std::vector<float> & percent,  std::vector<float> & y,  std::vector<short> & sectionType,  std::vector<float> & coeff)
{
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TTObjectBasePtr curve;
    TTValue         v;
    TTUInt32        i;
    TTErr           err;
    
    // get curve object at address
    err = timeProcess->sendMessage(TTSymbol("CurveGet"), toTTAddress(address), v);
    
    if (!err) {
        
        curve = v[0];
        
        // get a curve parameters
        err = curve->getAttributeValue(TTSymbol("parameters"), v);
        
        if (!err) {
            
            // edit percent, y, sectionType and coeff from v : x1 y1 b1 x2 y2 b2
            for (i = 0; i < v.size(); i = i+3) {
                
                percent.push_back(TTFloat64(v[i]) * 100.);
                y.push_back(TTFloat64(v[i+1]));
                sectionType.push_back(1);
                coeff.push_back(TTFloat64(v[i+2]));
            }
            
            sectionType.push_back(1);
            coeff.push_back(1);
        }
    }
    
    return err == kTTErrNone;
}

bool Engine::getCurveValues(TimeProcessId boxId, const std::string & address, unsigned int argNb, std::vector<float>& result)
{
    TimeProcessPtr  timeProcess = getTimeProcess(boxId);
    TTObjectBasePtr curve;
    TTValue         v, curveValues;
    TTErr           err;
    
    // get curve object at address
    err = timeProcess->sendMessage(TTSymbol("CurveGet"), toTTAddress(address), v);

    if (!err) {
        
        curve = v[0];
        
        // get time process duration
        timeProcess->getAttributeValue(TTSymbol("duration"), v);
    
        // sample the curve
        err = curve->sendMessage(TTSymbol("Sample"), v, curveValues);
    
        // copy the curveValues into the result vector
        for (TTUInt32 i = 0; i < curveValues.size(); i++)
            result.push_back(TTFloat64(curveValues[i]));
    }
    
	return err == kTTErrNone;
}

InteractiveProcessId Engine::addTriggerPoint(TimeProcessId containingBoxId, TimeEventIndex controlPointIndex)
{
    TimeProcessPtr          timeProcess = getTimeProcess(containingBoxId);
    TimeEventPtr            timeEvent;
    InteractiveProcessId    triggerId;
    TTValue                 v;
    
    // Replace a static event by an interactive one
    if (controlPointIndex == BEGIN_CONTROL_POINT_INDEX)
        timeProcess->sendMessage(TTSymbol("StartEventReplace"), TTSymbol("InteractiveEvent"), kTTValNONE);
    else
        timeProcess->sendMessage(TTSymbol("EndEventReplace"), TTSymbol("InteractiveEvent"), kTTValNONE);
    
    // We cache the time process and the event index instead of the event itself
    triggerId = cacheInteractiveProcess(timeProcess, controlPointIndex);
    
    // TODO : create a TTCallback to observe any time event attribute
    // getInteractiveEvent(triggerId)->appendObserver() ?
    
	return triggerId;
}

void Engine::removeTriggerPoint(InteractiveProcessId triggerId)
{
    TimeEventIndex controlPointIndex;
    TimeProcessPtr timeProcess = getInteractiveProcess(triggerId, controlPointIndex);
    
    // TODO : remove observers on the interactive event
    // getInteractiveEvent(triggerId)->removeObserver() ?
    
    // Replace an interactive event by a static one
    if (controlPointIndex == BEGIN_CONTROL_POINT_INDEX)
        timeProcess->sendMessage(TTSymbol("StartEventReplace"), TTSymbol("StaticEvent"), kTTValNONE);
    else
        timeProcess->sendMessage(TTSymbol("EndEventReplace"), TTSymbol("StaticEvent"), kTTValNONE);
    
    // Cache it and get an unique id for this event
    uncacheInteractiveProcess(triggerId);
}

void Engine::setTriggerPointMessage(InteractiveProcessId triggerId, std::string triggerMessage)
{
#ifdef TODO_ENGINE
	m_editor->setTriggerPointMessage(triggerId, triggerMessage);
#endif
    return;
}

std::string Engine::getTriggerPointMessage(InteractiveProcessId triggerId)
{
#ifdef TODO_ENGINE
	return m_editor->getTriggerPointMessage(triggerId);
#endif
    
    std::string empty;
    return empty;
}

TimeProcessId Engine::getTriggerPointRelatedBoxId(InteractiveProcessId triggerId)
{
#ifdef TODO_ENGINE
	return m_editor->getTriggerPointRelatedBoxId(triggerId);
#endif
    
    return NO_ID;
}

TimeEventIndex Engine::getTriggerPointRelatedCtrlPointIndex(InteractiveProcessId triggerId)
{
#ifdef TODO_ENGINE
	return m_editor->getTriggerPointRelatedControlPointIndex(triggerId);
#endif
    
    return 0;
}

void Engine::getBoxesId(vector<TimeProcessId>& boxesID)
{
#ifdef TODO_ENGINE    
	m_editor->getAllBoxesId(boxesID);
#endif
}

void Engine::getRelationsId(vector<IntervalId>& relationsID)
{
#ifdef TODO_ENGINE
	m_editor->getAllAntPostRelationsId(relationsID);
#endif
}

void Engine::getTriggersPointId(vector<InteractiveProcessId>& triggersID)
{
#ifdef TODO_ENGINE
	m_editor->getAllTriggersId(triggersID);
#endif
}

//Execution ///////////////////////////////////////////////////////////
void Engine::setGotoValue(TimeValue gotoValue)
{
#ifdef TODO_ENGINE    
	m_executionMachine->setGotoInformation(gotoValue);
#endif
}

TimeValue Engine::getGotoValue()
{
#ifdef TODO_ENGINE    
	return m_executionMachine->getGotoInformation();
#endif
}

bool Engine::play()
{
    // compile the main scenario
    m_mainScenario->sendMessage(TTSymbol("Compile"));
    
    return !m_mainScenario->sendMessage(TTSymbol("Play"));
}

void Engine::pause(bool pauseValue)
{
    if (pauseValue)
        m_mainScenario->sendMessage(TTSymbol("Pause"));
    else
        m_mainScenario->sendMessage(TTSymbol("resume"));;
}

bool Engine::isPaused()
{
    TTValue         v;
    TTObjectBasePtr aScheduler;
    
    // get the scheduler object of the main scenario
    m_mainScenario->getAttributeValue(TTSymbol("scheduler"), v);
    aScheduler = TTObjectBasePtr(v[0]);
    
    aScheduler->getAttributeValue(TTSymbol("running"), v);
    
    return TTBoolean(v[0]);
}

bool Engine::stop()
{
    return !m_mainScenario->sendMessage(TTSymbol("Stop"));
}

bool Engine::isRunning()
{
    TTValue         v;
    TTObjectBasePtr aScheduler;
    
    // get the scheduler object of the main scenario
    m_mainScenario->getAttributeValue(TTSymbol("scheduler"), v);
    aScheduler = TTObjectBasePtr(v[0]);
    
    aScheduler->getAttributeValue(TTSymbol("running"), v);
    
    return TTBoolean(v[0]);
}

TimeValue Engine::getCurrentExecutionTime()
{
    TTValue         v;
    TTUInt32        time;
    TTObjectBasePtr aScheduler;
    
    // get the scheduler object of the main scenario
    m_mainScenario->getAttributeValue(TTSymbol("scheduler"), v);
    aScheduler = TTObjectBasePtr(v[0]);
    
    aScheduler->getAttributeValue(TTSymbol("realTime"), v);
    time = TTFloat64(v[0]);
    
    return time;
}

void Engine::setExecutionSpeedFactor(float factor)
{
    TTValue         v;
    TTObjectBasePtr aScheduler;

    // get the scheduler object of the main scenario
    m_mainScenario->getAttributeValue(TTSymbol("scheduler"), v);
    aScheduler = TTObjectBasePtr(v[0]);
    
    aScheduler->setAttributeValue(TTSymbol("speed"), TTFloat64(factor));
}

float Engine::getExecutionSpeedFactor()
{
    TTValue         v;
    TTObjectBasePtr aScheduler;
    
    // get the scheduler object of the main scenario
    m_mainScenario->getAttributeValue(TTSymbol("scheduler"), v);
    aScheduler = TTObjectBasePtr(v[0]);
    
    aScheduler->getAttributeValue(TTSymbol("speed"), v);
    
    return TTFloat64(v[0]);
}

float Engine::getProcessProgression(TimeProcessId processId)
{
    TTValue         v;
    TTUInt32        time;
    TimeProcessPtr  timeProcess = getTimeProcess(processId);
    TTObjectBasePtr aScheduler;
    
    // get the scheduler object of the time process
    timeProcess->getAttributeValue(TTSymbol("scheduler"), v);
    aScheduler = TTObjectBasePtr(v[0]);
    
    aScheduler->getAttributeValue(TTSymbol("realTime"), v);
    time = TTFloat64(v[0]);
    
    return time;
}

bool Engine::receiveNetworkMessage(std::string netMessage)
{
#ifdef TODO_ENGINE    
	if (m_executionMachine->isRunning()) {
		return m_executionMachine->receiveNetworkMessage(netMessage);
	} else {
		return false;
	}
#endif
}

void Engine::simulateNetworkMessageReception(const std::string & netMessage)
{
#ifdef TODO_ENGINE    
	std::string value("");
	receiveNetworkMessageCallBack(this, netMessage, "", value);
    //	if (m_executionMachine->isRunning()) {
    //		m_executionMachine->receiveNetworkMessage(netMessage);
    //	}
#endif
}

void Engine::addNetworkDevice(const std::string & deviceName, const std::string & pluginToUse, const std::string & DeviceIp, const std::string & DevicePort)
{
    TTValue         v;
    TTSymbol        applicationName(deviceName);
    TTSymbol        protocolName(pluginToUse);
    TTObjectBasePtr anApplication = NULL;
    TTHash          hashParameters;
    
    // if the application doesn't already exist
    if (!getApplication(applicationName)) {
        
        // create the application
        v = applicationName;
        TTObjectBaseInstantiate(kTTSym_Application, TTObjectBaseHandle(&anApplication), v);
        
        // check if the protocol has been loaded
		if (getProtocol(protocolName)) {
            
            // register the application to the protocol
            v = applicationName;
            getProtocol(protocolName)->sendMessage(TTSymbol("registerApplication"), v, kTTValNONE);
            
            // set plugin parameters (OSC or Minuit Plugin)
            hashParameters.append(TTSymbol("ip"), TTSymbol(DeviceIp));
            
            v = TTSymbol(DevicePort);
            v.fromString();
            hashParameters.append(TTSymbol("port"), v);
            
            v = applicationName;
            v.append(TTPtr(&hashParameters));
            getProtocol(protocolName)->setAttributeValue(TTSymbol("applicationParameters"), v);
            
            // run the protocol for this application
            getProtocol(protocolName)->sendMessage(TTSymbol("Run"), applicationName, kTTValNONE);
        }
    }
}

void Engine::removeNetworkDevice(const std::string & deviceName)
{
    TTValue         v;
    TTSymbol        applicationName(deviceName);
    TTSymbol        protocolName;
    TTObjectBasePtr anApplication = getApplication(applicationName);
    
    // if the application exists
    if (anApplication) {
        
        // get the protocols of the application
        v = getApplicationProtocols(applicationName);
        protocolName = v[0]; // we register application to 1 protocol only
        
        // stop the protocol for this application
        getProtocol(protocolName)->sendMessage(TTSymbol("Stop"), applicationName, kTTValNONE);
        
        // unregister the application to the protocol
        v = TTValue(applicationName);
        getProtocol(protocolName)->sendMessage(TTSymbol("unregisterApplication"), v, kTTValNONE);
        
        // delete the application
        TTObjectBaseRelease(TTObjectBaseHandle(&anApplication));
    }
}

void Engine::sendNetworkMessage(const std::string & stringToSend)
{
#ifdef TODO_ENGINE
	m_networkController->deviceSendSetRequest(stringToSend);
#endif
}

void Engine::getNetworkDevicesName(std::vector<std::string>& devicesName, std::vector<bool>& couldSendNamespaceRequest)
{
    TTValue applicationNames, protocolNames;
    TTSymbol name;
    
    // get all application name
    TTModularApplications->getAttributeValue(TTSymbol("applicationNames"), applicationNames);
    
    for (TTUInt8 i = 0; i < applicationNames.size(); i++) {
        
        // don't return the local application
        name = applicationNames[i];
        
        if (name == getLocalApplicationName)
            continue;
        
        devicesName.push_back(name.c_str());
        
        // get all protocol names used by this application
        protocolNames = getApplicationProtocols(name);
        
        // if there is at least one protocol,
        if (protocolNames.size()) {
            
            // look if it provides namespace exploration
            name = protocolNames[0];
            couldSendNamespaceRequest.push_back(getProtocol(name)->mDiscover);
        }
    }
}

std::vector<std::string> Engine::requestNetworkSnapShot(const std::string & address)
{
    vector<string>      snapshot;
    TTAddress           anAddress = toTTAddress(address);
    TTSymbol            type;
    TTNodeDirectoryPtr  aDirectory;
    TTNodePtr           aNode;
    TTMirrorPtr         aMirror;
    TTList              nodeList;
    TTString            s;
    TTValue             v;
    
    // get the application directory
    aDirectory = getApplicationDirectory(anAddress.getDirectory());
    
    if (aDirectory) {
    
        // get the node
        if (!aDirectory->getTTNode(anAddress, &aNode)) {
            
            // get object attributes
            aMirror = TTMirrorPtr(aNode->getObject());
            if (aMirror) {
                
                type = aMirror->getName();
                
                if (type == TTSymbol("Data")) {
                    
                    // get the value attribute
                    aMirror->getAttributeValue(TTSymbol("value"), v);
                    v.toString();
                    s = TTString(v[0]);
                    
                    // append address value to the snapshot
                    snapshot.push_back(address + " " + s.data());
                }
            }
        }
    }
    
    return snapshot;
}

int Engine::requestNetworkNamespace(const std::string & address, vector<string>& nodes, vector<string>& leaves, vector<string>& attributs, vector<string>& attributsValue)
{
    TTAddress           anAddress = toTTAddress(address);
    TTSymbol            type;
    TTNodeDirectoryPtr  aDirectory;
    TTNodePtr           aNode, childNode;
    TTMirrorPtr         aMirror;
    TTList              nodeList;
    TTString            s;
    TTValue             v;
    
    // get the application directory
    aDirectory = getApplicationDirectory(anAddress.getDirectory());
    
    if (!aDirectory)
        return 0;
    
    // explore the directory at this address
    // notice the tree is already built (see in initModular)
    if (!aDirectory->getTTNode(anAddress, &aNode)) {
        
        // get object attributes
        aMirror = TTMirrorPtr(aNode->getObject());
        if (aMirror) {
            
            type = aMirror->getName();
            
            if (type == TTSymbol("Data")) {
                
                // append an unique value attribute
                attributs.push_back("value");
                
                // get the value attribute
                aMirror->getAttributeValue(TTSymbol("value"), v);
                v.toString();
                s = TTString(v[0]);
                attributsValue.push_back(s.c_str());
            }
        }
        
        // TODO : get attributes value
        
        // get children
        aNode->getChildren(S_WILDCARD, S_WILDCARD, nodeList);
        
        // sort children in leaves and nodes
        for (nodeList.begin(); nodeList.end(); nodeList.next()) {
            
            childNode = TTNodePtr(TTPtr(nodeList.current()[0]));
            
            // prepare name.instance to store
            s = childNode->getName().string();
            if (childNode->getInstance() != kTTSymEmpty) {
                s += ".";
                s += childNode->getInstance().string();
            }
            
            // depending on the mirror object type
            aMirror = TTMirrorPtr(childNode->getObject());
            if (aMirror) {
                
                type = aMirror->getName();
                
                if (type == TTSymbol("Data"))
                    leaves.push_back(s.c_str());
                else
                    nodes.push_back(s.c_str());
                    
            }
        }
        
        return 1;
    }
    
    return 0;
}



// LOAD AND STORE

void Engine::store(std::string fileName)
{
#ifdef TODO_ENGINE    
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
    
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST "ENGINES");
	xmlSetProp(root_node, BAD_CAST "version", BAD_CAST ENGINE_VERSION);
    
	std::ostringstream oss;
	oss << m_editor->scenarioSize();
	xmlSetProp(root_node, BAD_CAST "scenarioSize", BAD_CAST oss.str().data());
    
	xmlDocSetRootElement(doc, root_node);
    
	m_editor->store(root_node);
	m_executionMachine->store(root_node);
    
	xmlSaveFormatFileEnc(fileName.data(), doc, "UTF-8", 1);
    
	xmlFreeDoc(doc);
#endif
}

void Engine::load(std::string fileName)
{
#ifdef TODO_ENGINE    
	xmlNodePtr racine = NULL;
	xmlNodePtr n = NULL;
	xmlDocPtr doc = xmlReadFile(fileName.data(), "UTF-8", 0);
    
	racine = xmlDocGetRootElement(doc);
    
	xmlChar* version = xmlGetProp(racine, BAD_CAST "version");
	xmlChar* xmlScenarioSize = xmlGetProp(racine, BAD_CAST "scenarioSize");
    
	(void) version;
    
	int scenarioSize = XMLConversion::xmlCharToInt(xmlScenarioSize);
    
	reset(scenarioSize);
    
	for (n = racine->children; n != NULL; n = n->next) {
		if (n->type == XML_ELEMENT_NODE) {
			if (xmlStrEqual(n->name, BAD_CAST "EDITOR")) {
				std::cout << "LOAD: EDITOR" << std::endl;
				m_editor->load(n);
			} else if (xmlStrEqual(n->name, BAD_CAST "ECOMACHINE")) {
				std::cout << "LOAD: ECO-MACHINE" << std::endl;
				m_executionMachine->load(n, m_networkController);
			}
		}
	}
    
	xmlFreeDoc(doc);
#endif
}

// NETWORK
void Engine::print() {
#ifdef TODO_ENGINE    
	m_executionMachine->getPetriNet()->print();
#endif
}

void Engine::printExecutionInLinuxConsole()
{
#ifdef TODO_ENGINE    
	std::vector<TimeProcessId> boxesId;
    
	getBoxesId(boxesId);
    
	bool mustDisplay = true;
    
	while(isRunning()){
		if ((getCurrentExecutionTime()/60)%2 == 0) {
			if (mustDisplay) {
				std::system("clear");
				for (unsigned int i = 0; i < boxesId.size(); ++i) {
					unsigned int processPercent;
                    
					processPercent = getProcessProgression(boxesId[i]);
                    
					if ((processPercent > 0) && (processPercent < 99)) {
						std::cout << "[*";
                        
						for (unsigned j = 10; j <= processPercent; j += 10) {
							std::cout << "*";
						}
                        
						for (unsigned j = processPercent + 1; j <= 90; j += 10) {
							std::cout << ".";
						}
                        
						std::cout << "] -> (" << i << ") (" << processPercent << ")" << std::endl;
					}
				}
				mustDisplay = false;
			}
		} else {
			mustDisplay = true;
		}
		usleep(50);
	}
#endif
}

#if 0
#pragma mark -
#pragma mark Callback Methods
#endif

void InteractiveEventActiveAttributeCallback(TTPtr baton, const TTValue& value)
{
    TTValuePtr          b;
    EnginePtr           engine;
    InteractiveProcessId  triggerId;
    TTBoolean           active;
	
	// unpack baton (engine, triggerId)
	b = (TTValuePtr)baton;
	engine = EnginePtr((TTPtr)(*b)[0]);
    triggerId = InteractiveProcessId((*b)[0]);
	
	// Unpack data (active)
	active = value[0];
    
	if (engine->m_InteractiveEventActiveAttributeCallback != NULL)
		engine->m_InteractiveEventActiveAttributeCallback(triggerId, active);
}

void TimeProcessSchedulerRunningAttributeCallback(TTPtr baton, const TTValue& value)
{
    TTValuePtr          b;
    EnginePtr           engine;
    TimeProcessId       boxId;
    TTBoolean           running;
	
	// unpack baton (engine, boxId, controlPointId)
	b = (TTValuePtr)baton;
	engine = EnginePtr((TTPtr)(*b)[0]);
    boxId = TimeProcessId((*b)[0]);
	
	// Unpack data (running)
	running = value[0];
    
	if (engine->m_TimeProcessSchedulerRunningAttributeCallback != NULL)
		engine->m_TimeProcessSchedulerRunningAttributeCallback(boxId, running);
}

void TransportDataValueCallback(TTPtr baton, const TTValue& value)
{
    TTValuePtr          b;
    EnginePtr           engine;
    TTSymbol            transport;
	
	// unpack baton (engine, transport)
	b = (TTValuePtr)baton;
	engine = EnginePtr((TTPtr)(*b)[0]);
    transport = (*b)[0];
	
    if (transport == TTSymbol("Play"))
        engine->play();
    
    else if (transport == TTSymbol("Stop"))
        engine->stop();
    
    else if (transport == TTSymbol("Pause"))
        engine->pause(!engine->isPaused());
    
    else if (transport == TTSymbol("Rewind")) {
        engine->stop();
        engine->setGotoValue(0);
    }
    else if (transport == TTSymbol("StartPoint")) {
        
        if (value.size() == 1)
            if (value[0].type() == kTypeUInt32)
                engine->setGotoValue(value[0]);
        
    }
    else if (transport == TTSymbol("Speed")) {
        
        if (value.size() == 1)
            if (value[0].type() == kTypeFloat32)
                engine->setExecutionSpeedFactor(value[0]);
    }
    
    engine->m_TransportDataValueCallback(transport, value);
}

TTAddress Engine::toTTAddress(string networktreeAddress)
{
    TTSymbol            temp(networktreeAddress);
    TTAddress           address(temp);
    TTAddress           applicationName, anAddress;
    TTString            s;
    
    // split the address to get application name and then an address
    address.splitAt(0, applicationName, anAddress);
    
    // edit applicationName:/anAddress
    s = applicationName.string();
    s += S_DIRECTORY.string();
    
    if (anAddress != kTTAdrsEmpty)
        s += anAddress.string();
    
    return TTAddress(s);
}

std::string Engine::toNetworkTreeAddress(TTAddress aTTAddress)
{
    std::string s = aTTAddress.getDirectory().string().c_str();
    s += aTTAddress.normalize().string();
    
    return s;
}

#if 0
#pragma mark -
#pragma mark Miscellaneous Methods
#endif

// TODO : this should move into a TTModularAPI file
TTErr TTModularCreateData(TTSymbol service, TTValuePtr baton, TTFunctionWithBatonAndValue valueCallback, TTObjectBasePtr *returnedData)
{
    TTValue			args;
	TTObjectBasePtr	returnValueCallback;
	
	// prepare arguments
	returnValueCallback = NULL;
	TTObjectBaseInstantiate(TTSymbol("callback"), TTObjectBaseHandle(&returnValueCallback), kTTValNONE);
    
	returnValueCallback->setAttributeValue(kTTSym_baton, TTPtr(baton));
	returnValueCallback->setAttributeValue(kTTSym_function, TTPtr(valueCallback));
    
	args.append(returnValueCallback);
	
	args.append(service);
	
    // create data
	*returnedData = NULL;
	TTObjectBaseInstantiate(kTTSym_Data, TTObjectBaseHandle(returnedData), args);
	
	return kTTErrNone;
}

// TODO : this should move into a TTModularAPI file
TTErr TTModularRegisterObject(TTAddress address, TTObjectBasePtr object)
{
    TTNodePtr returnedTTNode;
    TTBoolean nodeCreated;
    
    return getLocalDirectory->TTNodeCreate(address, object, NULL, &returnedTTNode, &nodeCreated);
}