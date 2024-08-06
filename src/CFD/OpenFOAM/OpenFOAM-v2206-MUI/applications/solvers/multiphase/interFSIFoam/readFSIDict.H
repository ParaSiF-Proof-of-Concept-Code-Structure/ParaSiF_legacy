//Read fsi dictionary file if it exists
if (fsiDict.headerOk())
{
    const PtrList<entry> interfaceConfigurations
    (
        fsiDict.lookup("interfaceLists")
    );

    //Iterate through interface lists
    forAll(interfaceConfigurations, cC)
    {
        const entry& interfaceI = interfaceConfigurations[cC];
        const dictionary& interfaceIDict = interfaceI.dict();

        interfaceNames.append(static_cast<word>(interfaceIDict.dictName()));

        List<word> patchNamesLocal(interfaceIDict.lookup("PatchNames"));

        patchNames.append(patchNamesLocal);

        List<word> patch6DoFNamesLocal(interfaceIDict.lookup("Patch6DoFNames"));

        patch6DoFNames.append(patch6DoFNamesLocal);

    }

    if (!(fsiDict.readIfPresent("cplFSIMethod", cplMethod)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing cplFSIMethod entry" << exit(FatalIOError);
    }

    
    if (!(fsiDict.readIfPresent("forgetFSITime", forgetTime)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing forgetFSITime entry" << exit(FatalIOError);
    }

    word sendFSINameX;
    word sendFSINameY;
    word sendFSINameZ;
    word fetchFSINameX;
    word fetchFSINameY;
    word fetchFSINameZ;

    if (!(fsiDict.readIfPresent("sendFSINameX", sendFSINameX)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing sendFSINameX entry" << exit(FatalIOError);
    }
    if (!(fsiDict.readIfPresent("sendFSINameY", sendFSINameY)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing sendFSINameY entry" << exit(FatalIOError);
    }
    if (!(fsiDict.readIfPresent("sendFSINameZ", sendFSINameZ)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing sendFSINameZ entry" << exit(FatalIOError);
    }
    if (!(fsiDict.readIfPresent("fetchFSINameX", fetchFSINameX)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing fetchFSINameX entry" << exit(FatalIOError);
    }
    if (!(fsiDict.readIfPresent("fetchFSINameY", fetchFSINameY)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing fetchFSINameY entry" << exit(FatalIOError);
    }
    if (!(fsiDict.readIfPresent("fetchFSINameZ", fetchFSINameZ)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing fetchFSINameZ entry" << exit(FatalIOError);
    }
    sendNameX = sendFSINameX;
    sendNameY = sendFSINameY;
    sendNameZ = sendFSINameZ;
    fetchNameX = fetchFSINameX;
    fetchNameY = fetchFSINameY;
    fetchNameZ = fetchFSINameZ;

    if (!(fsiDict.readIfPresent("pushFSIForce", pushForce)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing pushFSIForce entry" << exit(FatalIOError);
    }

    if (!(fsiDict.readIfPresent("applyFSIFetchX", applyFetchX)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing applyFSIFetchX entry" << exit(FatalIOError);
    }

    if (!(fsiDict.readIfPresent("applyFSIFetchY", applyFetchY)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing applyFSIFetchY entry" << exit(FatalIOError);
    }

    if (!(fsiDict.readIfPresent("applyFSIFetchZ", applyFetchZ)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing applyFSIFetchZ entry" << exit(FatalIOError);
    }

    if (!(fsiDict.readIfPresent("searchAreaSampler", rSampler)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing searchAreaSampler entry" << exit(FatalIOError);
    }

    if (cplMethod != "Loose")
    {
        if (!(fsiDict.readIfPresent("initFSIUndRelxCpl", initUndRelxCpl)))
        {
            FatalIOErrorIn("", fsiDict)
                            << "Missing initFSIUndRelxCpl entry" << exit(FatalIOError);
        }
    }

            
    if (!(fsiDict.readIfPresent("pushFSIDelay", pushDelay)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing pushFSIDelay entry" << exit(FatalIOError);
    }

    if (!(fsiDict.readIfPresent("twoDimFSICorrection", twoDimCorrection)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing twoDimFSICorrection entry" << exit(FatalIOError);
    }

    if (twoDimCorrection)
    {
        if (!(fsiDict.readIfPresent("twoDimFSICellHeight", twoDimCellHeight)))
        {
            FatalIOErrorIn("", fsiDict)
                            << "Missing twoDimFSICellHeight entry" << exit(FatalIOError);
        }
    }

    if (!(fsiDict.readIfPresent("rigidMotionFrqX", rigidMotionFrqX_)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing rigidMotionFrqX entry" << exit(FatalIOError);
    }

    if (!(fsiDict.readIfPresent("rigidMotionFrqY", rigidMotionFrqY_)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing rigidMotionFrqY entry" << exit(FatalIOError);
    }

    if (!(fsiDict.readIfPresent("rigidMotionFrqZ", rigidMotionFrqZ_)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing rigidMotionFrqZ entry" << exit(FatalIOError);
    }

    if (!(fsiDict.readIfPresent("rigidMotionAmplitudeX", rigidMotionAmplitudeX_)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing rigidMotionAmplitudeX entry" << exit(FatalIOError);
    }

    if (!(fsiDict.readIfPresent("rigidMotionAmplitudeY", rigidMotionAmplitudeY_)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing rigidMotionAmplitudeY entry" << exit(FatalIOError);
    }

    if (!(fsiDict.readIfPresent("rigidMotionAmplitudeZ", rigidMotionAmplitudeZ_)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing rigidMotionAmplitudeZ entry" << exit(FatalIOError);
    }
    if (!(fsiDict.readIfPresent("rotation2DFrq", rotation2DFrq_)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing rotation2DFrq entry" << exit(FatalIOError);
    }

    if (!(fsiDict.readIfPresent("rotation2DAmplitude", rotation2DAmplitude_)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing rotation2DAmplitude entry" << exit(FatalIOError);
    }

    if (!(fsiDict.readIfPresent("rotation2DOriginX", rotation2DOriginX_)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing rotation2DOriginX entry" << exit(FatalIOError);
    }

    if (!(fsiDict.readIfPresent("rotation2DOriginY", rotation2DOriginY_)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing rotation2DOriginY entry" << exit(FatalIOError);
    }

    if (!(fsiDict.readIfPresent("rotation2DOriginZ", rotation2DOriginZ_)))
    {
        FatalIOErrorIn("", fsiDict)
                        << "Missing rotation2DOriginZ entry" << exit(FatalIOError);
    }
}
else
{
        FatalIOErrorIn("", fsiDict)
                        << "Missing fsiDict" << exit(FatalIOError);
}