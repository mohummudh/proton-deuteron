[] Check indexing - fix indexing
[] Visually inspect low chi-squared on proton hypothesis for kaons
[] Select kaons using the autoencoder
[] Validate results

NOTES

13/Feb/26

- Issue about bias is not in the training data protons being heavily weighted towards shorter protons. Issue is when we do the inference and get all the latent vectors out, regions which encode shorter activity/tracks will be more populated than the regions that encode longer activity/tracks. Therefore shorter regions will be higher density, and thus 'more probably a proton' in my current definition where more density = more likely to be proton. 
    - To solve this, I need to weight different regions of the latent space according to the frequency of protons, at least to account for the fact that there are more short protons, but this does not mean long protons are not protons just because there are less of them. 
    

