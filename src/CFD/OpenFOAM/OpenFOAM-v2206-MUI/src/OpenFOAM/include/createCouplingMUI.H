/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2018-2021 OpenCFD Ltd.
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM, distributed under GPL-3.0-or-later.

Description
    Create new MUI coupling interfaces (2D/3D/Templated)

Inputs
	3 extern declared DynamicLists to store MUI interface object pointers

Outputs
    3 populated DynamicLists containing pointers to 2D/3D/Templated MUI
    interface objects as described in associated "cpuplingDict" dictionary

\*---------------------------------------------------------------------------*/

{
    point meshMin(VGREAT, VGREAT, VGREAT);
    point meshMax(-VSMALL, -VSMALL, -VSMALL);

    const pointField& meshPoints = mesh.points();

    forAll(meshPoints, pts)
    {
        if(meshPoints[pts][0] < meshMin[0])
        {
            meshMin[0] = meshPoints[pts][0];
        }

        if(meshPoints[pts][1] < meshMin[1])
        {
            meshMin[1] = meshPoints[pts][1];
        }

        if(meshPoints[pts][2] < meshMin[2])
        {
            meshMin[2] = meshPoints[pts][2];
        }

        if(meshPoints[pts][0] > meshMax[0])
        {
            meshMax[0] = meshPoints[pts][0];
        }

        if(meshPoints[pts][1] > meshMax[1])
        {
            meshMax[1] = meshPoints[pts][1];
        }

        if(meshPoints[pts][2] > meshMax[2])
        {
            meshMax[2] = meshPoints[pts][2];
        }
    }

    //Determine mesh extents
    vector extents = meshMax - meshMin;

    string inputFile("couplingDict");

    IOdictionary couplingDict
    (
        IOobject
        (
            inputFile,
            runTime.system(),
            mesh,
            IOobject::READ_IF_PRESENT,
            IOobject::NO_WRITE
        )
    );

    word domainName;
    scalar refLength = 1.0;
    scalar refTime = 1.0;

    // 2D data
    std::vector<std::string> interfaceNames2d; //Interface name list in std:: for MUI API
    DynamicList<bool> send2d;
    DynamicList<vector> sendStart2d;
    DynamicList<vector> sendEnd2d;
    DynamicList<bool> receive2d;
    DynamicList<vector> receiveStart2d;
    DynamicList<vector> receiveEnd2d;
    DynamicList<bool> smartSend2d;
    DynamicList<bool> smartSendSync2d;
    DynamicList<bool> iterationCoupling2d;

    // 3D data
    std::vector<std::string> interfaceNames3d; //Interface name list in std:: for MUI API
    DynamicList<bool> send3d;
    DynamicList<vector> sendStart3d;
    DynamicList<vector> sendEnd3d;
    DynamicList<bool> receive3d;
    DynamicList<vector> receiveStart3d;
    DynamicList<vector> receiveEnd3d;
    DynamicList<bool> smartSend3d;
    DynamicList<bool> smartSendSync3d;
    DynamicList<bool> iterationCoupling3d;

    // Templated data
    std::vector<std::string> interfaceNamesT; //Interface name list in std:: for MUI API
    DynamicList<bool> sendT;
    DynamicList<vector> sendStartT;
    DynamicList<vector> sendEndT;
    DynamicList<bool> receiveT;
    DynamicList<vector> receiveStartT;
    DynamicList<vector> receiveEndT;
    DynamicList<bool> smartSendT;
    DynamicList<bool> smartSendSyncT;
    DynamicList<bool> iterationCouplingT;

    //Read coupling dictionary file if it exists
    if (couplingDict.headerOk())
    {
        if (!(couplingDict.readIfPresent("couplingName", domainName)))
        {
            FatalIOErrorIn("", couplingDict)
                           << "Missing couplingName entry" << exit(FatalIOError);
        }

        couplingDict.readIfPresent("refLength", refLength);
        couplingDict.readIfPresent("refTime", refTime);

        const PtrList<entry> couplingConfigurationsList
        (
            couplingDict.lookup("couplingConfigurations")
        );

        //Iterate through configuration lists (2D/3D/Templated)
        forAll(couplingConfigurationsList, cC)
        {
            const entry& couplingConfigI = couplingConfigurationsList[cC];
            const dictionary& couplingConfigIDict = couplingConfigI.dict();

            wordList iFaceTOC(couplingConfigIDict.toc());

            if(iFaceTOC.size() > 0)
            {
                if(couplingConfigIDict.dictName() == "TwoDInterfaces")
                {
                    interfaceNames2d.resize(iFaceTOC.size());
                    send2d.resize(iFaceTOC.size());
                    sendStart2d.resize(iFaceTOC.size());
                    sendEnd2d.resize(iFaceTOC.size());
                    receive2d.resize(iFaceTOC.size());
                    receiveStart2d.resize(iFaceTOC.size());
                    receiveEnd2d.resize(iFaceTOC.size());
                    smartSend2d.resize(iFaceTOC.size());
                    smartSendSync2d.resize(iFaceTOC.size());
                    iterationCoupling2d.resize(iFaceTOC.size());

                    //Iterate through interfaces
                    forAll(iFaceTOC, cI)
                    {
                        const dictionary& interfaceDict = couplingConfigIDict.subDict(iFaceTOC[cI]);

                        interfaceNames2d[cI] = static_cast<std::string>(interfaceDict.dictName());

                        if (interfaceDict.found("smartSend"))
                        {
                            smartSend2d[cI] = Switch(interfaceDict.lookup("smartSend"));
                        }
                        else
                        {
                            FatalIOErrorIn("", couplingDict)
                                           << "Missing interface smartSend entry" << exit(FatalIOError);
                        }

                        if( smartSend2d[cI] )
                        {
                            if (interfaceDict.found("smartSendSynchronise"))
                            {
                                smartSendSync2d[cI] = Switch(interfaceDict.lookup("smartSendSynchronise"));
                            }
                            else
                            {
                                FatalIOErrorIn("", couplingDict)
                                               << "Missing interface smartSendSynchronise entry when smartSend exists" << exit(FatalIOError);
                            }
                        }

                        if (interfaceDict.found("sending"))
                        {
                            send2d[cI] = Switch(interfaceDict.lookup("sending"));
                        }
                        else
                        {
                            FatalIOErrorIn("", couplingDict)
                                           << "Missing interface sending entry" << exit(FatalIOError);
                        }

                        // Smart send enabled and interface set to send so define domain values according to dictionary input
                        if(smartSend2d[cI] && send2d[cI])
                        {
                            bool sendDomDefined = false;

                            if (interfaceDict.found("domainSendStart"))
                            {
                                sendStart2d[cI] = vector(interfaceDict.lookup("domainSendStart"));
                                sendDomDefined = true;
                            }

                            if(sendDomDefined)
                            {
                                if (interfaceDict.found("domainSendEnd"))
                                {
                                    sendEnd2d[cI] = vector(interfaceDict.lookup("domainSendEnd"));
                                }
                                else
                                {
                                    FatalIOErrorIn("", couplingDict)
                                                   << "Missing corresponding interface domainSendEnd entry " << exit(FatalIOError);
                                }

                                // Both start and end defined for send domain so truncate extents to fit mesh bounds if region not completely outside mesh
                                if(sendStart2d[cI][0] < meshMax[0] && sendEnd2d[cI][0] > meshMin[0] &&
                                   sendStart2d[cI][1] < meshMax[1] && sendEnd2d[cI][1] > meshMin[1] &&
                                   sendStart2d[cI][2] < meshMax[2] && sendEnd2d[cI][2] > meshMin[2])
                                {
                                    if(sendStart2d[cI][0] < meshMin[0]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        sendStart2d[cI][0] = meshMin[0] - extents[0]*0.005;
                                    }

                                    if(sendEnd2d[cI][0] > meshMax[0]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        sendEnd2d[cI][0] = meshMax[0] + extents[0]*0.005;
                                    }

                                    if(sendStart2d[cI][1] < meshMin[1]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        sendStart2d[cI][1] = meshMin[1] - extents[1]*0.005;
                                    }

                                    if(sendEnd2d[cI][1] > meshMax[1]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        sendEnd2d[cI][1] = meshMax[1] + extents[1]*0.005;
                                    }

                                    if(sendStart2d[cI][2] < meshMin[2]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        sendStart2d[cI][2] = meshMin[2] - extents[2]*0.005;
                                    }

                                    if(sendEnd2d[cI][2] > meshMax[2]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        sendEnd2d[cI][2] = meshMax[2] + extents[2]*0.005;
                                    }
                                }
                                else //Set send region to mesh bounds (this mesh is outside of the sending region so need to avoid overlap with other regions this rank shouldn't interact with)
                                {
                                    sendStart2d[cI][0] = meshMin[0];
                                    sendStart2d[cI][1] = meshMin[1];
                                    sendStart2d[cI][2] = meshMin[2];
                                    sendEnd2d[cI][0] = meshMax[0];
                                    sendEnd2d[cI][1] = meshMax[1];
                                    sendEnd2d[cI][2] = meshMax[2];
                                }
                            }
                            else
                            {
                                if (interfaceDict.found("domainSendEnd"))
                                {
                                    FatalIOErrorIn("", couplingDict)
                                                   << "Missing corresponding interface domainSendStart entry " << exit(FatalIOError);
                                }

                                // "domain_send_start" and "domain_send_end" not set in dictionary, so initialise domain values to mesh bounds +/- 0.5%
                                sendStart2d[cI][0] = meshMin[0] - extents[0]*0.005;
                                sendStart2d[cI][1] = meshMin[1] - extents[1]*0.005;
                                sendStart2d[cI][2] = meshMin[2] - extents[2]*0.005;
                                sendEnd2d[cI][0] = meshMax[0] + extents[0]*0.005;
                                sendEnd2d[cI][1] = meshMax[1] + extents[1]*0.005;
                                sendEnd2d[cI][2] = meshMax[2] + extents[2]*0.005;
                            }
                        }
                        else // Smart send not enabled and/or interface not set to send so just initialise domain values to zero as they wont be used
                        {
                            sendStart2d[cI][0] = 0;
                            sendStart2d[cI][1] = 0;
                            sendStart2d[cI][2] = 0;
                            sendEnd2d[cI][0] = 0;
                            sendEnd2d[cI][1] = 0;
                            sendEnd2d[cI][2] = 0;
                        }

                        if (interfaceDict.found("receiving"))
                        {
                            receive2d[cI] = Switch(interfaceDict.lookup("receiving"));
                        }
                        else
                        {
                            FatalIOErrorIn("", couplingDict)
                                           << "Missing interface receiving entry" << exit(FatalIOError);
                        }

                        // Smart send enabled and interface set to receive so define domain values according to dictionary input
                        if(smartSend2d[cI] && receive2d[cI])
                        {
                            bool receiveDomDefined = false;

                            if (interfaceDict.found("domainReceiveStart"))
                            {
                                receiveStart2d[cI] = vector(interfaceDict.lookup("domainReceiveStart"));
                                receiveDomDefined = true;
                            }

                            if(receiveDomDefined)
                            {
                                if (interfaceDict.found("domainReceiveEnd"))
                                {
                                    receiveEnd2d[cI] = vector(interfaceDict.lookup("domainReceiveEnd"));
                                }
                                else
                                {
                                    FatalIOErrorIn("", couplingDict)
                                                   << "Missing corresponding interface domainReceiveEnd entry " << exit(FatalIOError);
                                }

                                // Both start and end defined for receive domain so truncate extents to fit mesh bounds if region not completely outside mesh
                                if(receiveStart2d[cI][0] < meshMax[0] && receiveEnd2d[cI][0] > meshMin[0] &&
                                   receiveStart2d[cI][1] < meshMax[1] && receiveEnd2d[cI][1] > meshMin[1] &&
                                   receiveStart2d[cI][2] < meshMax[2] && receiveEnd2d[cI][2] > meshMin[2])
                                {
                                    if(receiveStart2d[cI][0] < meshMin[0]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        receiveStart2d[cI][0] = meshMin[0] - extents[0]*0.005;
                                    }

                                    if(receiveEnd2d[cI][0] > meshMax[0]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        receiveEnd2d[cI][0] = meshMax[0] + extents[0]*0.005;
                                    }

                                    if(receiveStart2d[cI][1] < meshMin[1]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        receiveStart2d[cI][1] = meshMin[1] - extents[1]*0.005;
                                    }

                                    if(receiveEnd2d[cI][1] > meshMax[1]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        receiveEnd2d[cI][1] = meshMax[1] + extents[1]*0.005;
                                    }

                                    if(receiveStart2d[cI][2] < meshMin[2]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        receiveStart2d[cI][2] = meshMin[2] - extents[2]*0.005;
                                    }

                                    if(receiveEnd2d[cI][2] > meshMax[2]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        receiveEnd2d[cI][2] = meshMax[2] + extents[2]*0.005;
                                    }
                                }
                                else  //Set receive region to mesh bounds (this mesh is outside of the sending region so need to avoid overlap with other regions this rank shouldn't interact with)
                                {
                                    receiveStart2d[cI][0] = meshMin[0];
                                    receiveStart2d[cI][1] = meshMin[1];
                                    receiveStart2d[cI][2] = meshMin[2];
                                    receiveEnd2d[cI][0] = meshMax[0];
                                    receiveEnd2d[cI][1] = meshMax[1];
                                    receiveEnd2d[cI][2] = meshMax[2];
                                }
                            }
                            else
                            {
                                if (interfaceDict.found("domainReceiveEnd"))
                                {
                                    FatalIOErrorIn("", couplingDict)
                                                   << "Missing corresponding interface domainReceiveStart entry " << exit(FatalIOError);
                                }

                                // "domain_receive_start" and "domain_receive_end" not set in dictionary, so initialise domain values to mesh bounds +/- 0.5%
                                receiveStart2d[cI][0] = meshMin[0] - extents[0]*0.005;
                                receiveStart2d[cI][1] = meshMin[1] - extents[1]*0.005;
                                receiveStart2d[cI][2] = meshMin[2] - extents[2]*0.005;
                                receiveEnd2d[cI][0] = meshMax[0] + extents[0]*0.005;
                                receiveEnd2d[cI][1] = meshMax[1] + extents[1]*0.005;
                                receiveEnd2d[cI][2] = meshMax[2] + extents[2]*0.005;
                            }
                        }
                        else // Smart send not enabled and/or interface not set to send so just initialise domain values to zero as they wont be used
                        {
                            receiveStart2d[cI][0] = 0;
                            receiveStart2d[cI][1] = 0;
                            receiveStart2d[cI][2] = 0;
                            receiveEnd2d[cI][0] = 0;
                            receiveEnd2d[cI][1] = 0;
                            receiveEnd2d[cI][2] = 0;
                        }

                        if (interfaceDict.found("iterationCoupling"))
                        {
                            iterationCoupling2d[cI] = Switch(interfaceDict.lookup("iterationCoupling"));
                        }
                        else
                        {
                            FatalIOErrorIn("", couplingDict)
                                           << "Missing interface iterationCoupling entry" << exit(FatalIOError);
                        }
                    }
                }

                if(couplingConfigIDict.dictName() == "ThreeDInterfaces")
                {
                    interfaceNames3d.resize(iFaceTOC.size());
                    send3d.resize(iFaceTOC.size());
                    sendStart3d.resize(iFaceTOC.size());
                    sendEnd3d.resize(iFaceTOC.size());
                    receive3d.resize(iFaceTOC.size());
                    receiveStart3d.resize(iFaceTOC.size());
                    receiveEnd3d.resize(iFaceTOC.size());
                    smartSend3d.resize(iFaceTOC.size());
                    smartSendSync3d.resize(iFaceTOC.size());
                    iterationCoupling3d.resize(iFaceTOC.size());

                    //Iterate through interfaces
                    forAll(iFaceTOC, cI)
                    {
                        const dictionary& interfaceDict = couplingConfigIDict.subDict(iFaceTOC[cI]);

                        interfaceNames3d[cI] = static_cast<std::string>(interfaceDict.dictName());

                        if (interfaceDict.found("smartSend"))
                        {
                            smartSend3d[cI] = Switch(interfaceDict.lookup("smartSend"));
                        }
                        else
                        {
                            FatalIOErrorIn("", couplingDict)
                                           << "Missing interface smartSend entry" << exit(FatalIOError);
                        }

                        if( smartSend3d[cI] )
                        {
                            if (interfaceDict.found("smartSendSynchronise"))
                            {
                                smartSendSync3d[cI] = Switch(interfaceDict.lookup("smartSendSynchronise"));
                            }
                            else
                            {
                                FatalIOErrorIn("", couplingDict)
                                              << "Missing interface smartSendSynchronise entry when smartSend exists" << exit(FatalIOError);
                            }
                        }

                        if (interfaceDict.found("sending"))
                        {
                            send3d[cI] = Switch(interfaceDict.lookup("sending"));
                        }
                        else
                        {
                            FatalIOErrorIn("", couplingDict)
                                           << "Missing interface sending entry" << exit(FatalIOError);
                        }

                        // Smart send enabled and interface set to send so define domain values according to dictionary input
                        if(smartSend3d[cI] && send3d[cI])
                        {
                            bool sendDomDefined = false;

                            if (interfaceDict.found("domainSendStart"))
                            {
                                sendStart3d[cI] = vector(interfaceDict.lookup("domainSendStart"));
                                sendDomDefined = true;
                            }

                            if(sendDomDefined)
                            {
                                if (interfaceDict.found("domainSendEnd"))
                                {
                                    sendEnd3d[cI] = vector(interfaceDict.lookup("domainSendEnd"));
                                }
                                else
                                {
                                    FatalIOErrorIn("", couplingDict)
                                                   << "Missing corresponding interface domainSendEnd entry " << exit(FatalIOError);
                                }

                                // Both start and end defined for send domain so truncate extents to fit mesh bounds if region not completely outside mesh
                                if(sendStart3d[cI][0] < meshMax[0] && sendEnd3d[cI][0] > meshMin[0] &&
                                   sendStart3d[cI][1] < meshMax[1] && sendEnd3d[cI][1] > meshMin[1] &&
                                   sendStart3d[cI][2] < meshMax[2] && sendEnd3d[cI][2] > meshMin[2])
                                {
                                    if(sendStart3d[cI][0] < meshMin[0]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        sendStart3d[cI][0] = meshMin[0] - extents[0]*0.005;
                                    }

                                    if(sendEnd3d[cI][0] > meshMax[0]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        sendEnd3d[cI][0] = meshMax[0] + extents[0]*0.005;
                                    }

                                    if(sendStart3d[cI][1] < meshMin[1]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        sendStart3d[cI][1] = meshMin[1] - extents[1]*0.005;
                                    }

                                    if(sendEnd3d[cI][1] > meshMax[1]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        sendEnd3d[cI][1] = meshMax[1] + extents[1]*0.005;
                                    }

                                    if(sendStart3d[cI][2] < meshMin[2]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        sendStart3d[cI][2] = meshMin[2] - extents[2]*0.005;
                                    }

                                    if(sendEnd3d[cI][2] > meshMax[2]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        sendEnd3d[cI][2] = meshMax[2] + extents[2]*0.005;
                                    }
                                }
                                else //Set send region to mesh bounds (this mesh is outside of the sending region so need to avoid overlap with other regions this rank shouldn't interact with)
                                {
                                    sendStart3d[cI][0] = meshMin[0];
                                    sendStart3d[cI][1] = meshMin[1];
                                    sendStart3d[cI][2] = meshMin[2];
                                    sendEnd3d[cI][0] = meshMax[0];
                                    sendEnd3d[cI][1] = meshMax[1];
                                    sendEnd3d[cI][2] = meshMax[2];
                                }
                            }
                            else
                            {
                                if (interfaceDict.found("domainSendEnd"))
                                {
                                    FatalIOErrorIn("", couplingDict)
                                                   << "Missing corresponding interface domainSendStart entry " << exit(FatalIOError);
                                }

                                // "domain_send_start" and "domain_send_end" not set in dictionary, so initialise domain values to mesh bounds +/- 0.5%
                                sendStart3d[cI][0] = meshMin[0] - extents[0]*0.005;
                                sendStart3d[cI][1] = meshMin[1] - extents[1]*0.005;
                                sendStart3d[cI][2] = meshMin[2] - extents[2]*0.005;
                                sendEnd3d[cI][0] = meshMax[0] + extents[0]*0.005;
                                sendEnd3d[cI][1] = meshMax[1] + extents[1]*0.005;
                                sendEnd3d[cI][2] = meshMax[2] + extents[2]*0.005;
                            }
                        }
                        else // Smart send not enabled and/or interface not set to send so just initialise domain values to zero as they wont be used
                        {
                            sendStart3d[cI][0] = 0;
                            sendStart3d[cI][1] = 0;
                            sendStart3d[cI][2] = 0;
                            sendEnd3d[cI][0] = 0;
                            sendEnd3d[cI][1] = 0;
                            sendEnd3d[cI][2] = 0;
                        }

                        if (interfaceDict.found("receiving"))
                        {
                            receive3d[cI] = Switch(interfaceDict.lookup("receiving"));
                        }
                        else
                        {
                            FatalIOErrorIn("", couplingDict)
                                           << "Missing interface receiving entry" << exit(FatalIOError);
                        }

                        // Smart send enabled and interface set to receive so define domain values according to dictionary input
                        if(smartSend3d[cI] && receive3d[cI])
                        {
                            bool receiveDomDefined = false;

                            if (interfaceDict.found("domainReceiveStart"))
                            {
                                receiveStart3d[cI] = vector(interfaceDict.lookup("domainReceiveStart"));
                                receiveDomDefined = true;
                            }

                            if(receiveDomDefined)
                            {
                                if (interfaceDict.found("domainReceiveEnd"))
                                {
                                    receiveEnd3d[cI] = vector(interfaceDict.lookup("domainReceiveEnd"));
                                }
                                else
                                {
                                    FatalIOErrorIn("", couplingDict)
                                                   << "Missing corresponding interface domainReceiveEnd entry " << exit(FatalIOError);
                                }

                                // Both start and end defined for receive domain so truncate extents to fit mesh bounds if region not completely outside mesh
                                if(receiveStart3d[cI][0] < meshMax[0] && receiveEnd3d[cI][0] > meshMin[0] &&
                                   receiveStart3d[cI][1] < meshMax[1] && receiveEnd3d[cI][1] > meshMin[1] &&
                                   receiveStart3d[cI][2] < meshMax[2] && receiveEnd3d[cI][2] > meshMin[2])
                                {
                                    if(receiveStart3d[cI][0] < meshMin[0]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        receiveStart3d[cI][0] = meshMin[0] - extents[0]*0.005;
                                    }

                                    if(receiveEnd3d[cI][0] > meshMax[0]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        receiveEnd3d[cI][0] = meshMax[0] + extents[0]*0.005;
                                    }

                                    if(receiveStart3d[cI][1] < meshMin[1]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        receiveStart3d[cI][1] = meshMin[1] - extents[1]*0.005;
                                    }

                                    if(receiveEnd3d[cI][1] > meshMax[1]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        receiveEnd3d[cI][1] = meshMax[1] + extents[1]*0.005;
                                    }

                                    if(receiveStart3d[cI][2] < meshMin[2]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        receiveStart3d[cI][2] = meshMin[2] - extents[2]*0.005;
                                    }

                                    if(receiveEnd3d[cI][2] > meshMax[2]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        receiveEnd3d[cI][2] = meshMax[2] + extents[2]*0.005;
                                    }
                                }
                                else  //Set receive region to mesh bounds (this mesh is outside of the sending region so need to avoid overlap with other regions this rank shouldn't interact with)
                                {
                                    receiveStart3d[cI][0] = meshMin[0];
                                    receiveStart3d[cI][1] = meshMin[1];
                                    receiveStart3d[cI][2] = meshMin[2];
                                    receiveEnd3d[cI][0] = meshMax[0];
                                    receiveEnd3d[cI][1] = meshMax[1];
                                    receiveEnd3d[cI][2] = meshMax[2];
                                }
                            }
                            else
                            {
                                if (interfaceDict.found("domainReceiveEnd"))
                                {
                                    FatalIOErrorIn("", couplingDict)
                                                   << "Missing corresponding interface domainReceiveStart entry " << exit(FatalIOError);
                                }

                                // "domain_receive_start" and "domain_receive_end" not set in dictionary, so initialise domain values to mesh bounds +/- 0.5%
                                receiveStart3d[cI][0] = meshMin[0] - extents[0]*0.005;
                                receiveStart3d[cI][1] = meshMin[1] - extents[1]*0.005;
                                receiveStart3d[cI][2] = meshMin[2] - extents[2]*0.005;
                                receiveEnd3d[cI][0] = meshMax[0] + extents[0]*0.005;
                                receiveEnd3d[cI][1] = meshMax[1] + extents[1]*0.005;
                                receiveEnd3d[cI][2] = meshMax[2] + extents[2]*0.005;
                            }
                        }
                        else // Smart send not enabled and/or interface not set to send so just initialise domain values to zero as they wont be used
                        {
                            receiveStart3d[cI][0] = 0;
                            receiveStart3d[cI][1] = 0;
                            receiveStart3d[cI][2] = 0;
                            receiveEnd3d[cI][0] = 0;
                            receiveEnd3d[cI][1] = 0;
                            receiveEnd3d[cI][2] = 0;
                        }

                        if (interfaceDict.found("iterationCoupling"))
                        {
                            iterationCoupling3d[cI] = Switch(interfaceDict.lookup("iterationCoupling"));
                        }
                        else
                        {
                            FatalIOErrorIn("", couplingDict)
                                           << "Missing interface iterationCoupling entry" << exit(FatalIOError);
                        }
                    }
                }

                if(couplingConfigIDict.dictName() == "TemplatedInterfaces")
                {
                    interfaceNamesT.resize(iFaceTOC.size());
                    sendT.resize(iFaceTOC.size());
                    sendStartT.resize(iFaceTOC.size());
                    sendEndT.resize(iFaceTOC.size());
                    receiveT.resize(iFaceTOC.size());
                    receiveStartT.resize(iFaceTOC.size());
                    receiveEndT.resize(iFaceTOC.size());
                    smartSendT.resize(iFaceTOC.size());
                    smartSendSyncT.resize(iFaceTOC.size());
                    iterationCouplingT.resize(iFaceTOC.size());

                    //Iterate through interfaces
                    forAll(iFaceTOC, cI)
                    {
                        const dictionary& interfaceDict = couplingConfigIDict.subDict(iFaceTOC[cI]);

                        interfaceNamesT[cI] = static_cast<std::string>(interfaceDict.dictName());

                        if (interfaceDict.found("smartSend"))
                        {
                            smartSendT[cI] = Switch(interfaceDict.lookup("smartSend"));
                        }
                        else
                        {
                            FatalIOErrorIn("", couplingDict)
                                           << "Missing interface smartSend entry" << exit(FatalIOError);
                        }

                        if( smartSendT[cI] )
                        {
                            if (interfaceDict.found("smartSendSynchronise"))
                            {
                                smartSendSyncT[cI] = Switch(interfaceDict.lookup("smartSendSynchronise"));
                            }
                            else
                            {
                                FatalIOErrorIn("", couplingDict)
                                              << "Missing interface smartSendSynchronise entry when smartSend exists" << exit(FatalIOError);
                            }
                        }

                        if (interfaceDict.found("sending"))
                        {
                            sendT[cI] = Switch(interfaceDict.lookup("sending"));
                        }
                        else
                        {
                            FatalIOErrorIn("", couplingDict)
                                           << "Missing interface sending entry" << exit(FatalIOError);
                        }

                        // Smart send enabled and interface set to send so define domain values according to dictionary input
                        if(smartSendT[cI] && sendT[cI])
                        {
                            bool sendDomDefined = false;

                            if (interfaceDict.found("domainSendStart"))
                            {
                                sendStartT[cI] = vector(interfaceDict.lookup("domainSendStart"));
                                sendDomDefined = true;
                            }

                            if(sendDomDefined)
                            {
                                if (interfaceDict.found("domainSendEnd"))
                                {
                                    sendEndT[cI] = vector(interfaceDict.lookup("domainSendEnd"));
                                }
                                else
                                {
                                    FatalIOErrorIn("", couplingDict)
                                                   << "Missing corresponding interface domainSendEnd entry " << exit(FatalIOError);
                                }

                                // Both start and end defined for send domain so truncate extents to fit mesh bounds if region not completely outside mesh
                                if(sendStartT[cI][0] < meshMax[0] && sendEndT[cI][0] > meshMin[0] &&
                                   sendStartT[cI][1] < meshMax[1] && sendEndT[cI][1] > meshMin[1] &&
                                   sendStartT[cI][2] < meshMax[2] && sendEndT[cI][2] > meshMin[2])
                                {
                                    if(sendStartT[cI][0] < meshMin[0]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        sendStartT[cI][0] = meshMin[0] - extents[0]*0.005;
                                    }

                                    if(sendEndT[cI][0] > meshMax[0]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        sendEndT[cI][0] = meshMax[0] + extents[0]*0.005;
                                    }

                                    if(sendStartT[cI][1] < meshMin[1]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        sendStartT[cI][1] = meshMin[1] - extents[1]*0.005;
                                    }

                                    if(sendEndT[cI][1] > meshMax[1]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        sendEndT[cI][1] = meshMax[1] + extents[1]*0.005;
                                    }

                                    if(sendStartT[cI][2] < meshMin[2]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        sendStartT[cI][2] = meshMin[2] - extents[2]*0.005;
                                    }

                                    if(sendEndT[cI][2] > meshMax[2]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        sendEndT[cI][2] = meshMax[2] + extents[2]*0.005;
                                    }
                                }
                                else //Set send region to mesh bounds (this mesh is outside of the sending region so need to avoid overlap with other regions this rank shouldn't interact with)
                                {
                                    sendStartT[cI][0] = meshMin[0];
                                    sendStartT[cI][1] = meshMin[1];
                                    sendStartT[cI][2] = meshMin[2];
                                    sendEndT[cI][0] = meshMax[0];
                                    sendEndT[cI][1] = meshMax[1];
                                    sendEndT[cI][2] = meshMax[2];
                                }
                            }
                            else
                            {
                                if (interfaceDict.found("domainSendEnd"))
                                {
                                    FatalIOErrorIn("", couplingDict)
                                                   << "Missing corresponding interface domainSendStart entry " << exit(FatalIOError);
                                }

                                // "domain_send_start" and "domain_send_end" not set in dictionary, so initialise domain values to mesh bounds +/- 0.5%
                                sendStartT[cI][0] = meshMin[0] - extents[0]*0.005;
                                sendStartT[cI][1] = meshMin[1] - extents[1]*0.005;
                                sendStartT[cI][2] = meshMin[2] - extents[2]*0.005;
                                sendEndT[cI][0] = meshMax[0] + extents[0]*0.005;
                                sendEndT[cI][1] = meshMax[1] + extents[1]*0.005;
                                sendEndT[cI][2] = meshMax[2] + extents[2]*0.005;
                            }
                        }
                        else // Smart send not enabled and/or interface not set to send so just initialise domain values to zero as they wont be used
                        {
                            sendStartT[cI][0] = 0;
                            sendStartT[cI][1] = 0;
                            sendStartT[cI][2] = 0;
                            sendEndT[cI][0] = 0;
                            sendEndT[cI][1] = 0;
                            sendEndT[cI][2] = 0;
                        }

                        if (interfaceDict.found("receiving"))
                        {
                            receiveT[cI] = Switch(interfaceDict.lookup("receiving"));
                        }
                        else
                        {
                            FatalIOErrorIn("", couplingDict)
                                           << "Missing interface receiving entry" << exit(FatalIOError);
                        }

                        // Smart send enabled and interface set to receive so define domain values according to dictionary input
                        if(smartSendT[cI] && receiveT[cI])
                        {
                            bool receiveDomDefined = false;

                            if (interfaceDict.found("domainReceiveStart"))
                            {
                                receiveStartT[cI] = vector(interfaceDict.lookup("domainReceiveStart"));
                                receiveDomDefined = true;
                            }

                            if(receiveDomDefined)
                            {
                                if (interfaceDict.found("domainReceiveEnd"))
                                {
                                    receiveEndT[cI] = vector(interfaceDict.lookup("domainReceiveEnd"));
                                }
                                else
                                {
                                    FatalIOErrorIn("", couplingDict)
                                                   << "Missing corresponding interface domainReceiveEnd entry " << exit(FatalIOError);
                                }

                                // Both start and end defined for receive domain so truncate extents to fit mesh bounds if region not completely outside mesh
                                if(receiveStartT[cI][0] < meshMax[0] && receiveEndT[cI][0] > meshMin[0] &&
                                   receiveStartT[cI][1] < meshMax[1] && receiveEndT[cI][1] > meshMin[1] &&
                                   receiveStartT[cI][2] < meshMax[2] && receiveEndT[cI][2] > meshMin[2])
                                {
                                    if(receiveStartT[cI][0] < meshMin[0]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        receiveStartT[cI][0] = meshMin[0] - extents[0]*0.005;
                                    }

                                    if(receiveEndT[cI][0] > meshMax[0]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        receiveEndT[cI][0] = meshMax[0] + extents[0]*0.005;
                                    }

                                    if(receiveStartT[cI][1] < meshMin[1]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        receiveStartT[cI][1] = meshMin[1] - extents[1]*0.005;
                                    }

                                    if(receiveEndT[cI][1] > meshMax[1]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        receiveEndT[cI][1] = meshMax[1] + extents[1]*0.005;
                                    }

                                    if(receiveStartT[cI][2] < meshMin[2]) //Snap to mesh bound minimum - 0.5%
                                    {
                                        receiveStartT[cI][2] = meshMin[2] - extents[2]*0.005;
                                    }

                                    if(receiveEndT[cI][2] > meshMax[2]) //Snap to mesh bound maximum + 0.5%
                                    {
                                        receiveEndT[cI][2] = meshMax[2] + extents[2]*0.005;
                                    }
                                }
                                else  //Set receive region to mesh bounds (this mesh is outside of the sending region so need to avoid overlap with other regions this rank shouldn't interact with)
                                {
                                    receiveStartT[cI][0] = meshMin[0];
                                    receiveStartT[cI][1] = meshMin[1];
                                    receiveStartT[cI][2] = meshMin[2];
                                    receiveEndT[cI][0] = meshMax[0];
                                    receiveEndT[cI][1] = meshMax[1];
                                    receiveEndT[cI][2] = meshMax[2];
                                }
                            }
                            else
                            {
                                if (interfaceDict.found("domainReceiveEnd"))
                                {
                                    FatalIOErrorIn("", couplingDict)
                                                   << "Missing corresponding interface domainReceiveStart entry " << exit(FatalIOError);
                                }

                                // "domain_receive_start" and "domain_receive_end" not set in dictionary, so initialise domain values to mesh bounds +/- 0.5%
                                receiveStartT[cI][0] = meshMin[0] - extents[0]*0.005;
                                receiveStartT[cI][1] = meshMin[1] - extents[1]*0.005;
                                receiveStartT[cI][2] = meshMin[2] - extents[2]*0.005;
                                receiveEndT[cI][0] = meshMax[0] + extents[0]*0.005;
                                receiveEndT[cI][1] = meshMax[1] + extents[1]*0.005;
                                receiveEndT[cI][2] = meshMax[2] + extents[2]*0.005;
                            }
                        }
                        else // Smart send not enabled and/or interface not set to send so just initialise domain values to zero as they wont be used
                        {
                            receiveStartT[cI][0] = 0;
                            receiveStartT[cI][1] = 0;
                            receiveStartT[cI][2] = 0;
                            receiveEndT[cI][0] = 0;
                            receiveEndT[cI][1] = 0;
                            receiveEndT[cI][2] = 0;
                        }

                        if (interfaceDict.found("iterationCoupling"))
                        {
                            iterationCouplingT[cI] = Switch(interfaceDict.lookup("iterationCoupling"));
                        }
                        else
                        {
                            FatalIOErrorIn("", couplingDict)
                                           << "Missing interface iterationCoupling entry" << exit(FatalIOError);
                        }
                    }
                }
            }
        }
    }

    couplingDict.close();

    // If there was at least one interface in the dictionary the ncheck MPI initialised
    if(interfaceNames2d.size() > 0 || interfaceNames3d.size() > 0 || interfaceNamesT.size() > 0)
    {
        // If this is not a parallel run then need to call MPI_Init through MUI (otherwise this is called during PStream creation)
        if (!args.parRunControl().parRun())
        {
            mui::mpi_split_by_app(argc, argv);
        }
    }

    // If there was at least one 2D interface in the dictionary then proceed to creation and set up
    if(interfaceNames2d.size() > 0)
    {
        label extentsDir = -1; //0 = 3D; 1 = Y/Z; 2 = X/Z; 3 = X/Y

        if(extents[0] >= (0.0 - VSMALL) && extents[0] <= (0.0 + VSMALL)) // X extent is zero, 2D plane is in Y/Z
        {
            extentsDir = 1;
        }
        else if(extents[1] >= (0.0 - VSMALL) && extents[1] <= (0.0 + VSMALL)) // Y extent is zero, 2D plane is in X/Z
        {
            extentsDir = 2;
        }
        else if(extents[2] >= (0.0 - VSMALL) && extents[2] <= (0.0 + VSMALL)) // Z extent is zero, 2D plane is in X/Y
        {
            extentsDir = 3;
        }
        else // Mesh is 3D
        {
            extentsDir = 0;
        }

        if (extentsDir == 0) //Mesh is 3D, inform that creating a 2D interface is not possible and exit
        {
            IOWarningIn("", couplingDict)
                << "Creating a 2D interface in a 3D mesh,"
                << " if SmartSend is enabled and the send/receive domain sizes"
                << " are not defined then the direction"
                << " of the interface will be assumed in the Y/Z plane"
                << endl;

            extentsDir = 1;
        }

        mui2dInterfaces.resize(interfaceNames2d.size());

        // Create MUI interface(s) using API helper function - interfaces returned in order stored in "interfaceNames2d"
        std::vector<std::unique_ptr<mui::uniface<mui::config_2d>>> returnInterfaces2d =
            mui::create_uniface<mui::config_2d>(static_cast<std::string>(domainName), interfaceNames2d);

        // Release raw pointer into main DynamicList and perform SmartSend announcements
        for(size_t i = 0; i < returnInterfaces2d.size(); i++)
        {
            mui2dInterfaces[i] = returnInterfaces2d[i].release();

            // Perform SmartSend announcements if enabled
            if(smartSend2d[i])
            {
                scalar oneOverRefLength = 1.0 / refLength;

                mui::point<mui::config_2d::REAL, mui::config_2d::D> start;
                mui::point<mui::config_2d::REAL, mui::config_2d::D> end;

                // Interface is set to send
                if(send2d[i])
                {
                    if(extentsDir == 1) //Extents in Y/Z
                    {
                        start[0] = sendStart2d[i][1];
                        start[1] = sendStart2d[i][2];

                        end[0] = sendEnd2d[i][1];
                        end[1] = sendEnd2d[i][2];
                    }
                    else if (extentsDir == 2) //Extents in X/Z
                    {
                        start[0] = sendStart2d[i][0];
                        start[1] = sendStart2d[i][2];

                        end[0] = sendEnd2d[i][0];
                        end[1] = sendEnd2d[i][2];
                    }
                    else if (extentsDir == 3) //Extents in X/Y
                    {
                        start[0] = sendStart2d[i][0];
                        start[1] = sendStart2d[i][1];

                        end[0] = sendEnd2d[i][0];
                        end[1] = sendEnd2d[i][1];
                    }

                    start[0] *= oneOverRefLength;
                    start[1] *= oneOverRefLength;
                    end[0] *= oneOverRefLength;
                    end[1] *= oneOverRefLength;

                    mui::geometry::box<mui::config_2d> region_2d(start, end);

                    //- Using iteration based coupling
                    if(iterationCoupling2d[i])
                    {
                        label totalIter = label((runTime.endTime().value() - runTime.startTime().value()) / runTime.deltaT().value());
                        mui2dInterfaces[i]->announce_send_span(0, totalIter, region_2d, smartSendSync2d[i]);
                    }
                    else //- Using direct time based coupling
                    {
                        mui2dInterfaces[i]->announce_send_span((runTime.startTime().value() / refTime),
                                                               (runTime.endTime().value() / refTime), region_2d, smartSendSync2d[i]);
                    }
                }

                //Interface is set to receive
                if(receive2d[i])
                {
                    if(extentsDir == 1) //Extents in Y/Z
                    {
                        start[0] = receiveStart2d[i][1];
                        start[1] = receiveStart2d[i][2];

                        end[0] = receiveEnd2d[i][1];
                        end[1] = receiveEnd2d[i][2];
                    }
                    else if (extentsDir == 2) //Extents in X/Z
                    {
                        start[0] = receiveStart2d[i][0];
                        start[1] = receiveStart2d[i][2];

                        end[0] = receiveEnd2d[i][0];
                        end[1] = receiveEnd2d[i][2];
                    }
                    else if (extentsDir == 3) //Extents in X/Y
                    {
                        start[0] = receiveStart2d[i][0];
                        start[1] = receiveStart2d[i][1];

                        end[0] = receiveEnd2d[i][0];
                        end[1] = receiveEnd2d[i][1];
                    }

                    start[0] *= oneOverRefLength;
                    start[1] *= oneOverRefLength;
                    end[0] *= oneOverRefLength;
                    end[1] *= oneOverRefLength;

                    mui::geometry::box<mui::config_2d> region_2d(start, end);

                    //- Using iteration based coupling
                    if(iterationCoupling2d[i])
                    {
                        label totalIter = label((runTime.endTime().value() - runTime.startTime().value()) / runTime.deltaT().value());
                        mui2dInterfaces[i]->announce_recv_span(0, totalIter, region_2d, smartSendSync2d[i]);
                    }
                    else //- Using direct time based coupling
                    {
                        mui2dInterfaces[i]->announce_recv_span((runTime.startTime().value() / refTime),
                                                               (runTime.endTime().value() / refTime), region_2d, smartSendSync2d[i]);
                    }
                }
            }
        }
    }
    // If there was at least one 3D interface in the dictionary then proceed to creation and set up
    if(interfaceNames3d.size() > 0)
    {
        mui3dInterfaces.resize(interfaceNames3d.size());

        // Create MUI interface(s) using API helper function - interfaces returned in order stored in "interfaceNames3d"
        std::vector<std::unique_ptr<mui::uniface<mui::config_3d>>> returnInterfaces3d =
            mui::create_uniface<mui::config_3d>(static_cast<std::string>(domainName), interfaceNames3d);
        // Release raw pointer into main DynamicList and perform SmartSend announcements
        for(size_t i = 0; i < returnInterfaces3d.size(); i++)
        {
            mui3dInterfaces[i] = returnInterfaces3d[i].release();

            // Perform SmartSend announcements if enabled
            if(smartSend3d[i])
            {
                scalar oneOverRefLength = 1.0 / refLength;

                //Interface is set to send
                if(send3d[i])
                {
                    //Create a 3D box geometry based on pre-computed send domain
                    mui::point<mui::config_3d::REAL, mui::config_3d::D> start(sendStart3d[i][0], sendStart3d[i][1], sendStart3d[i][2]);
                    mui::point<mui::config_3d::REAL, mui::config_3d::D> end(sendEnd3d[i][0], sendEnd3d[i][1], sendEnd3d[i][2]);

                    start[0] *= oneOverRefLength;
                    start[1] *= oneOverRefLength;
                    start[2] *= oneOverRefLength;
                    end[0] *= oneOverRefLength;
                    end[1] *= oneOverRefLength;
                    end[2] *= oneOverRefLength;

                    mui::geometry::box<mui::config_3d> region_3d(start, end);

                    //- Using iteration based coupling
                    if(iterationCoupling3d[i])
                    {
                        label totalIter = label((runTime.endTime().value() - runTime.startTime().value()) / runTime.deltaT().value());
                        mui3dInterfaces[i]->announce_send_span(0, totalIter, region_3d, smartSendSync3d[i]);
                    }
                    else //- Using direct time based coupling
                    {
                      mui3dInterfaces[i]->announce_send_span((runTime.startTime().value() / refTime),
                                                             (runTime.endTime().value() / refTime), region_3d, smartSendSync3d[i]);
                    }
                }

                //Interface is set to receive
                if(receive3d[i])
                {
                    //Create a 3D box geometry based on pre-computed receive domain
                    mui::point<mui::config_3d::REAL, mui::config_3d::D> start(receiveStart3d[i][0], receiveStart3d[i][1], receiveStart3d[i][2]);
                    mui::point<mui::config_3d::REAL, mui::config_3d::D> end(receiveEnd3d[i][0], receiveEnd3d[i][1], receiveEnd3d[i][2]);

                    start[0] *= oneOverRefLength;
                    start[1] *= oneOverRefLength;
                    start[2] *= oneOverRefLength;
                    end[0] *= oneOverRefLength;
                    end[1] *= oneOverRefLength;
                    end[2] *= oneOverRefLength;

                    mui::geometry::box<mui::config_3d> region_3d(start, end);

                    //- Using iteration based coupling
                    if(iterationCoupling3d[i])
                    {
                        label totalIter = label((runTime.endTime().value() - runTime.startTime().value()) / runTime.deltaT().value());
                        mui3dInterfaces[i]->announce_recv_span(0, totalIter, region_3d, smartSendSync3d[i]);
                    }
                    else //- Using direct time based coupling
                    {
                        mui3dInterfaces[i]->announce_recv_span((runTime.startTime().value() / refTime),
                                                               (runTime.endTime().value() / refTime), region_3d, smartSendSync3d[i]);
                    }
                }
            }
        }
    }

    // If there was at least one templated interface in the dictionary then proceed to creation and set up
    if(interfaceNamesT.size() > 0)
    {
        label extentsDir = -1; //0 = 3D; 1 = Y/Z; 2 = X/Z; 3 = X/Y

        if(mui::config_of::D == 2) //Treat for 2D interfaces
        {
            if(extents[0] >= (0.0 - VSMALL) && extents[0] <= (0.0 + VSMALL)) // X extent is zero, 2D plane is in Y/Z
            {
                extentsDir = 1;
            }
            else if(extents[1] >= (0.0 - VSMALL) && extents[1] <= (0.0 + VSMALL)) // Y extent is zero, 2D plane is in X/Z
            {
                extentsDir = 2;
            }
            else if(extents[2] >= (0.0 - VSMALL) && extents[2] <= (0.0 + VSMALL)) // Z extent is zero, 2D plane is in X/Y
            {
                extentsDir = 3;
            }
            else // Mesh is 3D
            {
                extentsDir = 0;
            }

            if (extentsDir == 0) //Mesh is 3D, inform that creating a 2D interface is not possible and exit
            {
                IOWarningIn("", couplingDict)
                    << "Creating a 2D templated interface in a 3D mesh,"
                    << " if SmartSend is enabled and the send/receive domain sizes"
                    << " are not defined then the direction"
                    << " of the interface will be assumed in the Y/Z plane"
                    << endl;

                extentsDir = 1;
            }
        }

        muiTemplatedInterfaces.resize(interfaceNamesT.size());

        // Create MUI interface(s) using API helper function - interfaces returned in order stored in "interfaceNamesT"
        std::vector<std::unique_ptr<mui::uniface<mui::config_of>>> returnInterfacesT =
            mui::create_uniface<mui::config_of>(static_cast<std::string>(domainName), interfaceNamesT);

        // Release raw pointer into main DynamicList and perform SmartSend announcements
        for(size_t i = 0; i < returnInterfacesT.size(); i++)
        {
            muiTemplatedInterfaces[i] = returnInterfacesT[i].release();

            // Perform SmartSend announcements if enabled
            if(smartSendT[i])
            {
                scalar oneOverRefLength = 1.0 / refLength;

                if(mui::config_of::D == 2) //Treat for 2D interfaces
                {
                    mui::point<mui::config_of::REAL, mui::config_of::D> start;
                    mui::point<mui::config_of::REAL, mui::config_of::D> end;

                    // Interface is set to send
                    if(sendT[i])
                    {
                        if(extentsDir == 1) //Extents in Y/Z
                        {
                            start[0] = sendStartT[i][1];
                            start[1] = sendStartT[i][2];

                            end[0] = sendEndT[i][1];
                            end[1] = sendEndT[i][2];
                        }
                        else if (extentsDir == 2) //Extents in X/Z
                        {
                            start[0] = sendStartT[i][0];
                            start[1] = sendStartT[i][2];

                            end[0] = sendEndT[i][0];
                            end[1] = sendEndT[i][2];
                        }
                        else if (extentsDir == 3) //Extents in X/Y
                        {
                            start[0] = sendStartT[i][0];
                            start[1] = sendStartT[i][1];

                            end[0] = sendEndT[i][0];
                            end[1] = sendEndT[i][1];
                        }

                        start[0] *= oneOverRefLength;
                        start[1] *= oneOverRefLength;
                        end[0] *= oneOverRefLength;
                        end[1] *= oneOverRefLength;

                        mui::geometry::box<mui::config_of> region_2d(start, end);

                        //- Using iteration based coupling
                        if(iterationCouplingT[i])
                        {
                            label totalIter = label((runTime.endTime().value() - runTime.startTime().value()) / runTime.deltaT().value());
                            muiTemplatedInterfaces[i]->announce_send_span(0, totalIter, region_2d, smartSendSyncT[i]);
                        }
                        else //- Using direct time based coupling
                        {
                          muiTemplatedInterfaces[i]->announce_send_span((runTime.startTime().value() / refTime),
                                                                        (runTime.endTime().value() / refTime), region_2d, smartSendSyncT[i]);
                        }
                    }

                    //Interface is set to receive
                    if(receiveT[i])
                    {
                        if(extentsDir == 1) //Extents in Y/Z
                        {
                            start[0] = receiveStartT[i][1];
                            start[1] = receiveStartT[i][2];

                            end[0] = receiveEndT[i][1];
                            end[1] = receiveEndT[i][2];
                        }
                        else if (extentsDir == 2) //Extents in X/Z
                        {
                            start[0] = receiveStartT[i][0];
                            start[1] = receiveStartT[i][2];

                            end[0] = receiveEndT[i][0];
                            end[1] = receiveEndT[i][2];
                        }
                        else if (extentsDir == 3) //Extents in X/Y
                        {
                            start[0] = receiveStartT[i][0];
                            start[1] = receiveStartT[i][1];

                            end[0] = receiveEndT[i][0];
                            end[1] = receiveEndT[i][1];
                        }

                        start[0] *= oneOverRefLength;
                        start[1] *= oneOverRefLength;
                        end[0] *= oneOverRefLength;
                        end[1] *= oneOverRefLength;

                        mui::geometry::box<mui::config_of> region_2d(start, end);

                        //- Using iteration based coupling
                        if(iterationCouplingT[i])
                        {
                            label totalIter = label((runTime.endTime().value() - runTime.startTime().value()) / runTime.deltaT().value());
                            muiTemplatedInterfaces[i]->announce_recv_span(0, totalIter, region_2d, smartSendSyncT[i]);
                        }
                        else //- Using direct time based coupling
                        {
                          muiTemplatedInterfaces[i]->announce_recv_span((runTime.startTime().value() / refTime),
                                                                        (runTime.endTime().value() / refTime), region_2d, smartSendSyncT[i]);
                        }
                    }
                }
                else if(mui::config_of::D == 3) //Treat for 3D interfaces
                {
                    //Interface is set to send
                    if(sendT[i])
                    {
                        //Create a 3D box geometry based on pre-computed send domain
                        mui::point<mui::config_of::REAL, mui::config_of::D> start(sendStartT[i][0], sendStartT[i][1], sendStartT[i][2]);
                        mui::point<mui::config_of::REAL, mui::config_of::D> end(sendEndT[i][0], sendEndT[i][1], sendEndT[i][2]);

                        start[0] *= oneOverRefLength;
                        start[1] *= oneOverRefLength;
                        start[2] *= oneOverRefLength;
                        end[0] *= oneOverRefLength;
                        end[1] *= oneOverRefLength;
                        end[2] *= oneOverRefLength;

                        mui::geometry::box<mui::config_of> region_3d(start, end);

                        //- Using iteration based coupling
                        if(iterationCouplingT[i])
                        {
                            label totalIter = label((runTime.endTime().value() - runTime.startTime().value()) / runTime.deltaT().value());
                            muiTemplatedInterfaces[i]->announce_send_span(0, totalIter, region_3d, smartSendSyncT[i]);
                        }
                        else //- Using direct time based coupling
                        {
                            muiTemplatedInterfaces[i]->announce_send_span((runTime.startTime().value() / refTime),
                                                                          (runTime.endTime().value() / refTime), region_3d, smartSendSyncT[i]);
                        }
                    }

                    //Interface is set to receive
                    if(receiveT[i])
                    {
                        //Create a 3D box geometry based on pre-computed receive domain
                        mui::point<mui::config_of::REAL, mui::config_of::D> start(receiveStartT[i][0], receiveStartT[i][1], receiveStartT[i][2]);
                        mui::point<mui::config_of::REAL, mui::config_of::D> end(receiveEndT[i][0], receiveEndT[i][1], receiveEndT[i][2]);

                        start[0] *= oneOverRefLength;
                        start[1] *= oneOverRefLength;
                        start[2] *= oneOverRefLength;
                        end[0] *= oneOverRefLength;
                        end[1] *= oneOverRefLength;
                        end[2] *= oneOverRefLength;

                        mui::geometry::box<mui::config_of> region_3d(start, end);

                        //- Using iteration based coupling
                        if(iterationCouplingT[i])
                        {
                            label totalIter = label((runTime.endTime().value() - runTime.startTime().value()) / runTime.deltaT().value());
                            muiTemplatedInterfaces[i]->announce_recv_span(0, totalIter, region_3d, smartSendSyncT[i]);
                        }
                        else //- Using direct time based coupling
                        {
                            muiTemplatedInterfaces[i]->announce_recv_span((runTime.startTime().value() / refTime),
                                                                          (runTime.endTime().value() / refTime), region_3d, smartSendSyncT[i]);
                        }
                    }
                }
                else if(mui::config_of::D == 1) //Treat for 1D interfaces
                {
                    FatalIOErrorIn("", couplingDict)
                                   << "Creating a 1D templated interface is not supported" << exit(FatalIOError);
                }
                else //Anything else
                {
                    FatalIOErrorIn("", couplingDict)
                                   << "Creating a templated interface with dimensionality < 1 or > 3 is not supported" << exit(FatalIOError);
                }
            }
        }
    }
}
