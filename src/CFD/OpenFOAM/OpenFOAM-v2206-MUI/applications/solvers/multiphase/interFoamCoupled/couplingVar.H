int timeSteps = 0;

// Total iteration counter

// word cplMethod;

int subIterationNum;

    //- Whether to change the sub-iteration number
    bool changeSubIter;

    //- When to change the sub-iteration number [s].
    double changeSubIterTime;

    //- New sub-iteration number.
    int subIterationNumNew;

string fsiFile("fsiDict");
string fsiForceName("coupledForces");
dictionary fsiForceDict ;
    IOdictionary fsiDict
    (
        IOobject
        (
            fsiFile,
            runTime.system(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );



    if (fsiDict.headerOk())
    {
        if (!(fsiDict.readIfPresent("subIterationNumber", subIterationNum)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing subIterationNum entry" << exit(FatalIOError);
        }
        // subIterationNum = subIterationNumber;

        int newFSISubIterationNum;
        if (!(fsiDict.readIfPresent("newFSISubIterationNum", newFSISubIterationNum)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing newFSISubIterationNum entry" << exit(FatalIOError);
        }
        subIterationNumNew = newFSISubIterationNum;

        bool changeFSISubIter;
        if (!(fsiDict.readIfPresent("changeFSISubIter", changeFSISubIter)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing changeFSISubIter entry" << exit(FatalIOError);
        }
        changeSubIter = changeFSISubIter;

        
        fsiForceDict = fsiDict.subDict(fsiForceName);
        
        word cplFSIMethod;
        if (!(fsiDict.readIfPresent("cplFSIMethod", cplFSIMethod)))
        {
            FatalIOErrorIn("", fsiDict)
                           << "Missing cplFSIMethod entry" << exit(FatalIOError);
        }
        cplMethod = cplFSIMethod;

        
        if (cplMethod != "Loose")
        {
            double initFSIUndRelxCpl;
            if (!(fsiDict.readIfPresent("initFSIUndRelxCpl", initFSIUndRelxCpl)))
            {
                FatalIOErrorIn("", fsiDict)
                               << "Missing initFSIUndRelxCpl entry" << exit(FatalIOError);
            }
            initUndRelxCpl = initFSIUndRelxCpl;
        }

    }



