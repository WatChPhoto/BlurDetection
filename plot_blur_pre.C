{
  TString directory = "output/";

  const int nSurveys = 6;
  TString SurveyNames[nSurveys] = {
    "BarrelSurveyFar",
    "BarrelSurveyRings",
    "BottomCornerSurvey",
    "BottomSurvey",
    "TopCornerSurvey",
    "TopSurvey"
  };

  enum survey_enum {
    BarrelSurveyFar,
    BarrelSurveyRings,
    BottomCornerSurvey,
    BottomSurvey,
    TopCornerSurvey,
    TopSurvey
  };
  

  TString version = "";

  float BlurCutThresh = -150;

  TFile *OutputFile = new TFile(directory+"blur_trees.root", "RECREATE");
  
  TTree *tBlur[nSurveys];
  TTree *tBlurBurst[nSurveys];

  TH1F *h_blur[nSurveys];
  TH2F *h_blur_burst[nSurveys];
  TH2F *h_blur_burst_norm[nSurveys];

  TGraph *g_blur_rms[nSurveys];

  TCanvas *c_blur = new TCanvas("c_blur", "c_blur", 0, 0, 1400, 900);
  c_blur->Divide(3, 2);

  TCanvas *c_blur_burst = new TCanvas("c_blur_burst", "c_blur_burst", 0, 0, 1400, 900);
  c_blur_burst->Divide(3, 2);

  TCanvas *c_blur_burst_norm = new TCanvas("c_blur_burst_norm", "c_blur_burst_norm", 0, 0, 1400, 900);
  c_blur_burst_norm->Divide(3, 2);

  TCanvas *c_blur_burst_rms = new TCanvas("c_blur_burst_rms", "c_blur_burst_rms", 0, 0, 1400, 900);
  c_blur_burst_rms->Divide(3, 2);


  float imageID, blur;
  
  for (int isurvey=0; isurvey<nSurveys; isurvey++) {
    c_blur->cd(isurvey+1)->SetLogy(1);
    	
    TString filename = directory+SurveyNames[isurvey]+version+".txt";
    TString treename = SurveyNames[isurvey];

    int nBins = 100;
    h_blur[isurvey] = new TH1F("h_"+treename, SurveyNames[isurvey]+";#sigma^{2}(L);Number of Photos", nBins, 0,10000);


    tBlur[isurvey] = new TTree(treename, treename);
    tBlur[isurvey]->ReadFile(filename, "imageID:blur");

    tBlur[isurvey]->Project("h_"+treename, "blur");

    // Show overflow bin
    h_blur[isurvey]->SetBinContent(nBins, h_blur[isurvey]->GetBinContent(nBins)+h_blur[isurvey]->GetBinContent(nBins+1));

    h_blur[isurvey]->SetLineWidth(3);
    
    h_blur[isurvey]->Draw();

    h_blur[isurvey]->GetXaxis()->SetNdivisions(5, 2, 0);
		       
    tBlur[isurvey]->SetBranchAddress("imageID", &imageID);
    tBlur[isurvey]->SetBranchAddress("blur", &blur);

    const int nImagesPerBurst = 10;
    int nImages = 0;
    int ImageID[nImagesPerBurst] = {0};
    int CurrentBurst;
    int nBursts = tBlur[isurvey]->GetEntries()/nImagesPerBurst;
    
    float blurs[nImagesPerBurst];
    float averageBlur = 0;
    float rmsBlur[nImagesPerBurst] = {0};
    int nCutImages = 0;

    h_blur_burst[isurvey] = new TH2F("h_burst_"+treename, SurveyNames[isurvey]+";Burst;#sigma^{2}(L);Number of Photos", nBursts, 0, nBursts, nBins, 0,10000);
    h_blur_burst_norm[isurvey] = new TH2F("h_burst_norm_"+treename, SurveyNames[isurvey]+";Burst;#sigma^{2}(L) - #sigma^{2}_{avg}(L);Number of Photos", nBursts, 0, nBursts, nBins, -500, 500);
    g_blur_rms[isurvey] = new TGraph();
    g_blur_rms[isurvey]->SetName("g_burst_rms"+SurveyNames[isurvey]);
    g_blur_rms[isurvey]->SetTitle(SurveyNames[isurvey]);
    g_blur_rms[isurvey]->GetXaxis()->SetTitle("Burst");
    g_blur_rms[isurvey]->GetYaxis()->SetTitle("#sigma^{2}(L) RMS");
  
    tBlurBurst[isurvey] = new TTree(treename+"Burst", treename+"Burst");
    tBlurBurst[isurvey]->Branch("BurstID", &CurrentBurst, "CurrentBurst/I");
    tBlurBurst[isurvey]->Branch("AvgBlur", &averageBlur, "AvgBlur/F");
    tBlurBurst[isurvey]->Branch("ImageID", ImageID, Form("ImageID[%d]/I",nImagesPerBurst));
    tBlurBurst[isurvey]->Branch("ImageBlur", blurs, Form("ImageBlur[%d]/F",nImagesPerBurst));
    
    cout << endl << SurveyNames[isurvey] << endl;
    
    for (int iimage=0; iimage<tBlur[isurvey]->GetEntries(); iimage++) {

      tBlur[isurvey]->GetEntry(iimage);

      int BurstNumber = imageID/10000;
      int imageNumber = imageID - BurstNumber*10000;

      // Skip non-burst (single-shot) images
      if (imageID < 10000) continue;
      
      // Initialize
      if (!nImages) {
	nBursts = 0;
	averageBlur = 0;
	//rmsBlur = 0;
	nCutImages = 0;
	CurrentBurst = BurstNumber;
      }

      // Major image number changes (likely a new burst, but...)
      else if (BurstNumber != CurrentBurst) {

	// Exception when minor image exceeds 999, the major image number changes even though it's the same burst
	if (imageNumber != 1) {
	  
	  if (nImages != nImagesPerBurst) cout << "Warning: Unexpected number of images (" << nImages << ") in burst " << CurrentBurst << endl;

	  averageBlur /= nImages;
	  
	  //cout << CurrentBurst << " " << nImages << " " << averageBlur << endl;

	  float rmsBlurVal = 0;
	  for (int jimage=0; jimage<nImages; jimage++) {
	    
	    //h_blur_burst_norm[isurvey]->Fill(nBursts, (blurs[jimage]-averageBlur)/rmsBlur);
	    h_blur_burst_norm[isurvey]->Fill(nBursts, blurs[jimage]-averageBlur);

	    if (blurs[jimage]-averageBlur < BlurCutThresh)
	      nCutImages++;

	    rmsBlurVal += pow(rmsBlur[jimage]-averageBlur, 2);
	  }
	  
	  rmsBlurVal /= nImages;
	  rmsBlurVal = sqrt(rmsBlurVal);
	  g_blur_rms[isurvey]->SetPoint(nBursts, nBursts, rmsBlurVal/averageBlur);
	  
	  tBlurBurst[isurvey]->Fill();

	  
	  // Reset
	  for (int jimage=0; jimage<nImagesPerBurst; jimage++) {
	    ImageID[jimage] = 0;
	    blurs[jimage] = 0;
	    rmsBlur[jimage] = 0;
	  }
	  nImages = 0;
	  averageBlur = 0;
	  //rmsBlur = 0;
	  
	  nBursts++;
	}

	CurrentBurst = BurstNumber;

      }

      blurs[nImages] = blur;
      h_blur_burst[isurvey]->Fill(nBursts, blur);
      
      averageBlur += blur;
      rmsBlur[nImages] = blur;
      ImageID[nImages] = imageID;
      nImages++;
    }
    
    //cout << CurrentBurst << " " << nImages << " " << averageBlur << endl;

    c_blur_burst->cd(isurvey+1);
    h_blur_burst[isurvey]->Draw("");

    c_blur_burst_norm->cd(isurvey+1);
    h_blur_burst_norm[isurvey]->Draw("");
    h_blur_burst_norm[isurvey]->SetTitle(SurveyNames[isurvey]+Form(" (%.0f", (float)100*nCutImages/tBlur[isurvey]->GetEntries())+"% cut)");
      
    TLine *blur_cut = new TLine(0, BlurCutThresh, nBursts, BlurCutThresh);
    blur_cut->SetLineColor(kRed);
    blur_cut->Draw();

    c_blur_burst_rms->cd(isurvey+1);
    g_blur_rms[isurvey]->Draw("AP");
    
    ofstream ImageCutFile(directory+SurveyNames[isurvey]+"_blurcut.txt");
    for (int iburst=0; iburst<tBlurBurst[isurvey]->GetEntries(); iburst++) {

      tBlurBurst[isurvey]->GetEntry(iburst);

      int nUsedImages = 0;
      
      ImageCutFile << Form("%03d",CurrentBurst);
      for (int iimage=0; iimage<nImagesPerBurst; iimage++) {

	if (blurs[iimage] - averageBlur > BlurCutThresh) {
	  ImageCutFile << " " << Form("B%07d.JPG",ImageID[iimage]);
	  nUsedImages++;
	}
      }
      ImageCutFile << endl;

      if (nUsedImages < 4)
	cout << "Warning: Number of images (" << nUsedImages << ") < 4 for Burst " << CurrentBurst << endl;
      
    }
  }
  c_blur->cd(1);
  c_blur->Print(directory+"summary.png");

  c_blur_burst->cd(1);
  c_blur_burst->Print(directory+"summary_burst.png");

  c_blur_burst_norm->cd(1);
  c_blur_burst_norm->Print(directory+"summary_burst_norm.png");
  
  OutputFile->Write();

}
