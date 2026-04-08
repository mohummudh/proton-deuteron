# Active Code Path From `protons-kaons-vae-ends.ipynb`

This document linearises the notebook into a plain Markdown walkthrough of the code path that is actually used for the current proton-vs-kaon workflow.

It keeps:

- the data loading and geometric filtering
- the image preparation used for the `ends50` VAE inputs
- the saved VAE definition and inference path
- latent extraction and reconstruction-error calculation for proton train, proton validation, and kaon samples
- the current affine conditional normalising flow marked `# IN USE`
- the thresholding and chi2-aware kaon inspection
- the downstream diagnostics that still consume the current affine-flow outputs

It intentionally leaves out:

- the earlier, simpler conditional-flow block before `# IN USE`
- the later spline-flow experiments
- exploratory notebook plots that do not feed the active classifier

## Inputs and assumptions

The active path depends on two external pickle files and one saved model:

- `"/Volumes/easystore/proton-deuteron/col&ind/chi-col.pkl"`
- `"/Volumes/easystore/proton-deuteron/col&ind/chi-ind.pkl"`
- `"model_vae_ends50.pt"`

The notebook does not retrain the VAE on the active path. It loads the saved checkpoint and uses it to encode proton and kaon images into latent means.

## 1. Load the raw tables and apply the shared geometry cut

The collection-plane table (`col`) and induction-plane table (`ind`) are filtered together. Any event that fails the geometry requirement in either plane is removed from both tables so the two views stay aligned.

```python
import copy

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

import torch
import torch.nn as nn
import torch.nn.functional as F
from torch import optim
from torch.utils.data import DataLoader, Subset, TensorDataset

from sklearn.metrics import (
    confusion_matrix,
    precision_recall_fscore_support,
    roc_auc_score,
    roc_curve,
)
from sklearn.model_selection import train_test_split

from nflows import distributions, flows, transforms


col = pd.read_pickle("/Volumes/easystore/proton-deuteron/col&ind/chi-col.pkl")
ind = pd.read_pickle("/Volumes/easystore/proton-deuteron/col&ind/chi-ind.pkl")

removed_indices_col = col[
    ~((col["height"] > 15) & (col["height"] < 179) & (col["width"] < 1500))
].index
removed_indices_ind = ind[
    ~((ind["height"] > 15) & (ind["height"] < 179) & (ind["width"] < 1500))
].index

removed_indices = removed_indices_col.union(removed_indices_ind)

col = col[~col.index.isin(removed_indices)]
ind = ind[~ind.index.isin(removed_indices)]

protons = col[col["particle_type"] == "proton"].reset_index(drop=True)
kaons = col[col["particle_type"] == "kaon"].reset_index(drop=True)
```

## 2. Build the `ends50` image tensors

Only the last 50 wires are kept from each image. Each cropped image is then placed on a fixed-width canvas by aligning the horizontal position to the brightest pixel in the first retained row. After padding, both collection and induction views are downsampled to `48 x 48`, stacked into a two-channel tensor, and transformed with `log1p`.

The notebook defines a few helper functions that are not used later. Only the active preprocessing helpers are kept here.

```python
def cut_start(image, target=50):
    """Keep only the last `target` rows (wires) of the image."""
    return image[-target:, :]


def pad_image_batch_gpu(
    images_list,
    target_wh=(1502, 51),
    device="cuda",
    batch_size=32,
    cut_rows=None,
):
    """
    Pad a list of 2D images onto a fixed canvas.

    The horizontal offset is set from the brightest pixel in the first row,
    matching the notebook logic used for the current VAE inputs.
    """
    target_w, target_h = target_wh
    results = []

    for batch_start in range(0, len(images_list), batch_size):
        batch_end = min(batch_start + batch_size, len(images_list))
        batch_images = images_list[batch_start:batch_end]

        if cut_rows is not None:
            batch_images = [cut_start(img, target=cut_rows) for img in batch_images]

        padded_batch = torch.zeros(
            len(batch_images),
            target_h,
            target_w,
            dtype=torch.float32,
            device=device,
        )

        for i, img in enumerate(batch_images):
            img_tensor = torch.from_numpy(img).float().to(device)
            h, w = img.shape

            v = torch.argmax(img_tensor[0]).item()
            a = 751 - v

            y0 = 0
            x0 = max(0, min(a, target_w - w))
            y1 = y0 + h
            x1 = x0 + w

            padded_batch[i, y0:y1, x0:x1] = img_tensor

        results.extend(padded_batch.cpu().numpy())

    return results


prep_device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

p_c_list = col[col["particle_type"] == "proton"]["image_intensity"].tolist()
p_i_list = ind[ind["particle_type"] == "proton"]["image_intensity"].tolist()
k_c_list = col[col["particle_type"] == "kaon"]["image_intensity"].tolist()
k_i_list = ind[ind["particle_type"] == "kaon"]["image_intensity"].tolist()

p_c = np.array(pad_image_batch_gpu(p_c_list, device=prep_device, batch_size=64, cut_rows=50))
p_i = np.array(pad_image_batch_gpu(p_i_list, device=prep_device, batch_size=64, cut_rows=50))
k_c = np.array(pad_image_batch_gpu(k_c_list, device=prep_device, batch_size=64, cut_rows=50))
k_i = np.array(pad_image_batch_gpu(k_i_list, device=prep_device, batch_size=64, cut_rows=50))

p_c_tensor = torch.from_numpy(p_c).float().to(prep_device)
p_i_tensor = torch.from_numpy(p_i).float().to(prep_device)
k_c_tensor = torch.from_numpy(k_c).float().to(prep_device)
k_i_tensor = torch.from_numpy(k_i).float().to(prep_device)

p_c_d = F.interpolate(
    p_c_tensor.unsqueeze(1), size=(48, 48), mode="bilinear", align_corners=False
).squeeze(1).cpu().numpy()
p_i_d = F.interpolate(
    p_i_tensor.unsqueeze(1), size=(48, 48), mode="bilinear", align_corners=False
).squeeze(1).cpu().numpy()
k_c_d = F.interpolate(
    k_c_tensor.unsqueeze(1), size=(48, 48), mode="bilinear", align_corners=False
).squeeze(1).cpu().numpy()
k_i_d = F.interpolate(
    k_i_tensor.unsqueeze(1), size=(48, 48), mode="bilinear", align_corners=False
).squeeze(1).cpu().numpy()

p = np.stack([p_c_d, p_i_d], axis=1)
k = np.stack([k_c_d, k_i_d], axis=1)

p = torch.from_numpy(p).float()
k = torch.from_numpy(k).float()

p = torch.log1p(p)
k = torch.log1p(k)
```

## 3. Define the VAE and make the proton train/validation split

The active path uses an 80/20 split on the proton tensor. That split is used twice:

- first for VAE inference bookkeeping: proton-train latents versus proton-validation latents
- later for length-aligned bookkeeping when the conditional flow is trained and evaluated

The VAE architecture is the one used by the saved checkpoint `model_vae_ends50.pt`.

```python
class VAE(nn.Module):
    def __init__(self, input_hw=(240, 240), latent=8, p_enc=0.2):
        super().__init__()
        in_h, in_w = input_hw
        assert in_h % 16 == 0 and in_w % 16 == 0

        self.encoder = nn.Sequential(
            nn.Conv2d(2, 32, 5, stride=2, padding=2),
            nn.BatchNorm2d(32),
            nn.Softplus(),
            nn.Dropout2d(p_enc),
            nn.Conv2d(32, 64, 5, stride=2, padding=2),
            nn.BatchNorm2d(64),
            nn.Softplus(),
            nn.Dropout2d(p_enc),
            nn.Conv2d(64, 128, 5, stride=2, padding=2),
            nn.BatchNorm2d(128),
            nn.Softplus(),
            nn.Dropout2d(p_enc),
            nn.Conv2d(128, 256, 5, stride=2, padding=2),
            nn.BatchNorm2d(256),
            nn.Softplus(),
            nn.Dropout2d(p_enc),
        )

        self.h_enc = in_h // 16
        self.w_enc = in_w // 16
        flat_size = 256 * self.h_enc * self.w_enc

        self.fc_mu = nn.Linear(flat_size, latent)
        self.fc_logvar = nn.Linear(flat_size, latent)

        self.fc_dec = nn.Linear(latent, flat_size)

        self.decoder = nn.Sequential(
            nn.ConvTranspose2d(256, 128, 5, stride=2, padding=2, output_padding=1),
            nn.BatchNorm2d(128),
            nn.Softplus(),
            nn.ConvTranspose2d(128, 64, 5, stride=2, padding=2, output_padding=1),
            nn.BatchNorm2d(64),
            nn.Softplus(),
            nn.ConvTranspose2d(64, 32, 5, stride=2, padding=2, output_padding=1),
            nn.BatchNorm2d(32),
            nn.Softplus(),
            nn.ConvTranspose2d(32, 2, 5, stride=2, padding=2, output_padding=1),
            nn.Softplus(),
        )

    def encode(self, x):
        h = self.encoder(x)
        h = h.flatten(1)
        mu = self.fc_mu(h)
        logvar = self.fc_logvar(h)
        return mu, logvar

    def reparameterise(self, mu, logvar):
        std = torch.exp(0.5 * logvar)
        eps = torch.randn_like(std)
        return mu + eps * std

    def decode(self, z):
        h = self.fc_dec(z)
        h = h.view(z.size(0), 256, self.h_enc, self.w_enc)
        return self.decoder(h)

    def forward(self, x):
        mu, logvar = self.encode(x)
        z = self.reparameterise(mu, logvar)
        recon = self.decode(z)
        return recon, mu, logvar, z


all_indices = np.arange(len(p))
train_idx, val_idx = train_test_split(all_indices, test_size=0.2, random_state=42)

train_subset = Subset(p, train_idx)
val_subset = Subset(p, val_idx)
```

## 4. Load the saved VAE and extract latent means plus reconstruction errors

The notebook repeats the same inference loop three times: once for proton-train, once for proton-validation, and once for kaons. The only thing that changes is the input tensor. Here that repeated logic is collapsed into one helper without changing what is computed.

The important output is the encoder mean `mu`. That is the latent representation passed into the normalising flow.

```python
device_inf = torch.device("mps" if torch.backends.mps.is_available() else "cpu")

model = VAE(input_hw=(48, 48), latent=4).to(device_inf)
state_dict = torch.load("model_vae_ends50.pt", map_location=device_inf)
model.load_state_dict(state_dict)
model.eval()


def encode_dataset(model, x_data, device, batch_size=8):
    model.eval()
    model.to(device)

    X_np = x_data.cpu().numpy() if torch.is_tensor(x_data) else np.asarray(x_data, dtype=np.float32)
    N, C, H, W = X_np.shape

    recon_all = np.empty((N, C, H, W), dtype=np.float32)
    latent_vectors = np.empty((N, model.fc_mu.out_features), dtype=np.float32)

    with torch.no_grad():
        for i in range(0, N, batch_size):
            j = min(i + batch_size, N)
            xb = torch.tensor(X_np[i:j], dtype=torch.float32).to(device)

            recon, mu, logvar, z = model(xb)
            recon_all[i:j] = recon.cpu().numpy()
            latent_vectors[i:j] = mu.cpu().numpy()

            if device.type == "mps":
                try:
                    torch.mps.empty_cache()
                except Exception:
                    pass

    re_per_sample = ((recon_all - X_np) ** 2).mean(axis=(1, 2, 3))
    return X_np, recon_all, latent_vectors, re_per_sample


X_train = torch.stack([train_subset[i] for i in range(len(train_subset))])
X_val = torch.stack([val_subset[i] for i in range(len(val_subset))])

X_train_np, recon_all, latent_vectors, RE_per_sample = encode_dataset(model, X_train, device_inf)
X_val_np, recon_all_val, latent_vectors_val, RE_per_sample_val = encode_dataset(model, X_val, device_inf)
Xk_np, recon_all_k, latent_vectors_k, RE_per_sample_k = encode_dataset(model, k, device_inf)
```

At this point the active notebook state is:

- `latent_vectors`: proton latents for the proton training split
- `latent_vectors_val`: proton latents for the held-out proton validation split
- `latent_vectors_k`: kaon latents
- `RE_per_sample`, `RE_per_sample_val`, `RE_per_sample_k`: reconstruction errors for those three groups

## 5. Optional latent-space view used by one downstream plot

This does not feed the classifier, but it is the shortest clean way to reproduce the later plot that colors kaon candidates by flow log-probability.

```python
import umap

Z_all_plot = np.concatenate([latent_vectors, latent_vectors_k, latent_vectors_val], axis=0)
labels_plot = np.concatenate([
    np.zeros(len(latent_vectors), dtype=np.int32),
    np.ones(len(latent_vectors_k), dtype=np.int32),
    np.full(len(latent_vectors_val), 2, dtype=np.int32),
])

reducer = umap.UMAP(n_neighbors=30, min_dist=0.1)
Z_umap = reducer.fit_transform(Z_all_plot)
E_d = Z_umap[labels_plot == 1]
```

## 6. Current normalising flow in use

This is the normalising-flow block that the notebook marks with `# IN USE`. It is the current affine conditional flow, not the earlier simple version and not the later spline-flow experiments.

The logic is:

1. use VAE encoder means as the latent vectors
2. standardize latent dimensions using proton-train latents only
3. split the proton-train latents again into a flow-train and flow-validation split
4. condition on a nonlinear function of track length
5. train an affine autoregressive flow with random permutations, early stopping, weight decay, and gradient clipping
6. score proton-validation and kaon samples with `-log p(z | c(L))`

### 6.1 Prepare the latent and length arrays

```python
np.random.seed(42)
torch.manual_seed(42)

X_mu = np.asarray(latent_vectors, dtype=np.float32)
Z_mu = np.asarray(latent_vectors_val, dtype=np.float32)
Y_mu = np.asarray(latent_vectors_k, dtype=np.float32)

Lx_raw = protons.iloc[train_idx]["height"].to_numpy(dtype=np.float32)
Lz_raw = protons.iloc[val_idx]["height"].to_numpy(dtype=np.float32)
Ly_raw = kaons["height"].to_numpy(dtype=np.float32)

assert len(X_mu) == len(Lx_raw)
assert len(Z_mu) == len(Lz_raw)
assert len(Y_mu) == len(Ly_raw)
```

### 6.2 Standardize latent space and build the length context

The flow sees a three-component context:

- the standardized length
- the squared standardized length
- the log of the raw length

Both latent standardization and context standardization are fit only on the flow-training proton split.

```python
z_mean = X_mu.mean(axis=0, keepdims=True)
z_std = X_mu.std(axis=0, keepdims=True) + 1e-6

X_all = (X_mu - z_mean) / z_std
Z_all = (Z_mu - z_mean) / z_std
Y_all = (Y_mu - z_mean) / z_std

idx = np.arange(len(X_all))
idx_tr, idx_va = train_test_split(idx, test_size=0.10, random_state=42, shuffle=True)

X_tr, X_va = X_all[idx_tr], X_all[idx_va]
L_tr_raw, L_va_raw = Lx_raw[idx_tr], Lx_raw[idx_va]


def make_context(L_raw, L_mean, L_std, eps=1e-3):
    L_raw = np.asarray(L_raw, dtype=np.float32)
    L_scaled = (L_raw - L_mean) / (L_std + 1e-6)
    return np.stack([L_scaled, L_scaled**2, np.log(L_raw + eps)], axis=1).astype(np.float32)


L_mean = float(L_tr_raw.mean())
L_std = float(L_tr_raw.std() + 1e-6)

C_tr = make_context(L_tr_raw, L_mean, L_std)
C_va = make_context(L_va_raw, L_mean, L_std)
C_z = make_context(Lz_raw, L_mean, L_std)
C_y = make_context(Ly_raw, L_mean, L_std)

c_mean = C_tr.mean(axis=0, keepdims=True)
c_std = C_tr.std(axis=0, keepdims=True) + 1e-6

C_tr = (C_tr - c_mean) / c_std
C_va = (C_va - c_mean) / c_std
C_z = (C_z - c_mean) / c_std
C_y = (C_y - c_mean) / c_std
```

### 6.3 Build and train the affine conditional flow

This is the exact flow family the notebook uses in the current block:

- `latent_dim = 4`
- `context_dim = 3`
- `10` permutation-plus-affine-autoregressive blocks
- hidden size `128`

```python
latent_dim = X_tr.shape[1]
context_dim = C_tr.shape[1]
hidden_features = 128
num_transforms = 10

transform_list = []
for _ in range(num_transforms):
    transform_list.append(transforms.RandomPermutation(features=latent_dim))
    transform_list.append(
        transforms.autoregressive.MaskedAffineAutoregressiveTransform(
            features=latent_dim,
            hidden_features=hidden_features,
            context_features=context_dim,
        )
    )

transform = transforms.CompositeTransform(transform_list)
flow = flows.Flow(transform, distributions.StandardNormal([latent_dim]))

flow_device = torch.device(
    "cuda" if torch.cuda.is_available() else
    "mps" if torch.backends.mps.is_available() else
    "cpu"
)
flow = flow.float().to(flow_device)

train_loader = DataLoader(
    TensorDataset(torch.from_numpy(X_tr), torch.from_numpy(C_tr)),
    batch_size=256,
    shuffle=True,
)
val_loader = DataLoader(
    TensorDataset(torch.from_numpy(X_va), torch.from_numpy(C_va)),
    batch_size=256,
    shuffle=False,
)

optimizer = optim.Adam(flow.parameters(), lr=3e-4, weight_decay=1e-5)
max_epochs = 300
patience = 20
min_delta = 1e-4
grad_clip_norm = 5.0

best_state = None
best_val_nll = np.inf
best_epoch = 0
bad_epochs = 0
history = {"train_nll": [], "val_nll": []}

for epoch in range(1, max_epochs + 1):
    flow.train()
    train_sum, train_count = 0.0, 0

    for batch_z, batch_c in train_loader:
        batch_z = batch_z.to(flow_device)
        batch_c = batch_c.to(flow_device)

        optimizer.zero_grad(set_to_none=True)
        loss = -flow.log_prob(inputs=batch_z, context=batch_c).mean()
        loss.backward()
        torch.nn.utils.clip_grad_norm_(flow.parameters(), max_norm=grad_clip_norm)
        optimizer.step()

        bs = batch_z.size(0)
        train_sum += loss.item() * bs
        train_count += bs

    train_nll = train_sum / max(train_count, 1)

    flow.eval()
    val_sum, val_count = 0.0, 0
    with torch.no_grad():
        for batch_z, batch_c in val_loader:
            batch_z = batch_z.to(flow_device)
            batch_c = batch_c.to(flow_device)

            val_loss = -flow.log_prob(inputs=batch_z, context=batch_c).mean()
            bs = batch_z.size(0)
            val_sum += val_loss.item() * bs
            val_count += bs

    val_nll = val_sum / max(val_count, 1)

    history["train_nll"].append(train_nll)
    history["val_nll"].append(val_nll)

    if val_nll < (best_val_nll - min_delta):
        best_val_nll = val_nll
        best_epoch = epoch
        best_state = copy.deepcopy(flow.state_dict())
        bad_epochs = 0
    else:
        bad_epochs += 1
        if bad_epochs >= patience:
            print(
                f"Early stop at epoch {epoch} | "
                f"best epoch {best_epoch} | best val NLL {best_val_nll:.4f}"
            )
            break

if best_state is not None:
    flow.load_state_dict(best_state)

print(f"Loaded best checkpoint from epoch {best_epoch} (val NLL={best_val_nll:.4f})")

plt.figure(figsize=(8, 4))
plt.plot(history["train_nll"], label="Flow train NLL")
plt.plot(history["val_nll"], label="Flow val NLL")
plt.xlabel("Epoch")
plt.ylabel("NLL")
plt.title("Conditional flow training")
plt.grid(alpha=0.3)
plt.legend()
plt.tight_layout()
plt.show()
```

### 6.4 Score proton-validation and kaon samples

The score used downstream is the negative conditional log-likelihood:

`score = -log p(z | c(L))`

Higher score means "less proton-like" under the proton-trained flow.

```python
def flow_score(z_np, c_np):
    flow.eval()
    with torch.no_grad():
        z_t = torch.from_numpy(z_np.astype(np.float32)).to(flow_device)
        c_t = torch.from_numpy(c_np.astype(np.float32)).to(flow_device)
        logp = flow.log_prob(inputs=z_t, context=c_t).cpu().numpy()
    return -logp, logp


train_score, train_logp = flow_score(X_tr, C_tr)
val_score, val_logp = flow_score(X_va, C_va)
proton_score, proton_logp = flow_score(Z_all, C_z)
kaon_score, kaon_logp = flow_score(Y_all, C_y)
```

### 6.5 Calibration and diagnostic plots from the active block

```python
plt.figure(figsize=(8, 4))
plt.hist(train_logp, bins=100, density=True, alpha=0.6, label="Flow-train protons")
plt.hist(val_logp, bins=100, density=True, alpha=0.6, label="Flow-val protons")
plt.xlabel("log p(z | c(L))")
plt.ylabel("Density")
plt.title("Calibration: proton train vs val")
plt.grid(alpha=0.3)
plt.legend()
plt.tight_layout()
plt.show()


def bin_groups(lengths, scores, quantiles=(0, 0.2, 0.4, 0.6, 0.8, 1.0), min_count=10):
    edges = np.quantile(lengths, quantiles)
    edges = np.unique(edges)
    groups, labels = [], []
    for i in range(len(edges) - 1):
        lo, hi = edges[i], edges[i + 1]
        mask = (lengths >= lo) & ((lengths < hi) if i < len(edges) - 2 else (lengths <= hi))
        if mask.sum() >= min_count:
            groups.append(scores[mask])
            labels.append(f"{lo:.0f}-{hi:.0f}")
    return groups, labels


val_groups, val_labels = bin_groups(L_va_raw, val_score)
ka_groups, ka_labels = bin_groups(Ly_raw, kaon_score)

fig, ax = plt.subplots(1, 2, figsize=(14, 4), sharey=True)

if len(val_groups) > 0:
    ax[0].boxplot(val_groups, labels=val_labels, showfliers=False)
ax[0].set_title("Flow-val proton score by length bin")
ax[0].set_xlabel("Length bin")
ax[0].set_ylabel("Score = -log p(z|c)")
ax[0].tick_params(axis="x", rotation=45)
ax[0].grid(alpha=0.3)

if len(ka_groups) > 0:
    ax[1].boxplot(ka_groups, labels=ka_labels, showfliers=False)
ax[1].set_title("Kaon score by length bin")
ax[1].set_xlabel("Length bin")
ax[1].tick_params(axis="x", rotation=45)
ax[1].grid(alpha=0.3)

plt.tight_layout()
plt.show()


def context_from_lengths(L_raw):
    C = make_context(L_raw, L_mean, L_std)
    C = (C - c_mean) / c_std
    return C.astype(np.float32)


anchors = np.quantile(Lx_raw, [0.2, 0.5, 0.8]).astype(np.float32)
fig, axes = plt.subplots(1, len(anchors), figsize=(5 * len(anchors), 4), sharex=True, sharey=True)

for ax, L0 in zip(np.atleast_1d(axes), anchors):
    c0 = context_from_lengths(np.array([L0], dtype=np.float32))
    with torch.no_grad():
        z_samp = flow.sample(num_samples=400, context=torch.from_numpy(c0).to(flow_device)).cpu().numpy()
    if z_samp.ndim == 3:
        z_samp = z_samp[0]

    mask = np.abs(Lx_raw - L0) <= max(3.0, 0.15 * L_std)
    z_real = X_all[mask]

    ax.scatter(z_samp[:, 0], z_samp[:, 1], s=8, alpha=0.4, label="flow samples")
    if len(z_real) > 0:
        ax.scatter(z_real[:, 0], z_real[:, 1], s=8, alpha=0.4, label="real protons")
    ax.set_title(f"L≈{L0:.1f}")
    ax.set_xlabel("z[0]")
    ax.set_ylabel("z[1]")
    ax.grid(alpha=0.3)
    ax.legend()

plt.tight_layout()
plt.show()

plt.figure(figsize=(10, 6))
plt.scatter(Lz_raw, proton_score, alpha=0.6, s=15, c=proton_score, cmap="viridis")
plt.colorbar(label="Anomaly Score")
plt.xlabel("Track Length (L)")
plt.ylabel("Conditional Score: -log p(z | c(L))")
plt.title("Validation protons")
plt.grid(True, linestyle="--", alpha=0.5)
plt.show()

plt.figure(figsize=(10, 6))
plt.scatter(Ly_raw, kaon_score, alpha=0.6, s=15, c=kaon_score, cmap="viridis")
plt.colorbar(label="Anomaly Score")
plt.xlabel("Track Length (L)")
plt.ylabel("Conditional Score: -log p(z | c(L))")
plt.title("Kaon candidates")
plt.grid(True, linestyle="--", alpha=0.5)
plt.ylim(0, 1000)
plt.show()

plt.figure(figsize=(8, 4))
plt.hist(kaon_score, bins=300, alpha=0.7, label="Kaons")
plt.hist(proton_score, bins=300, alpha=0.7, label="Val protons")
plt.xlim(0, np.percentile(np.concatenate([kaon_score, proton_score]), 99.5))
plt.xlabel("Score = -log p(z|c)")
plt.ylabel("Count")
plt.title("Conditional flow score distributions")
plt.legend()
plt.grid(alpha=0.3)
plt.tight_layout()
plt.show()

plt.figure(figsize=(10, 5))
all_logp = np.concatenate([train_logp, val_logp, kaon_logp])
lo, hi = np.percentile(all_logp, [0.2, 99.8])
bins = np.linspace(lo, hi, 5000)

plt.hist(train_logp, bins=bins, density=True, alpha=0.55, label="Flow-train protons")
plt.hist(val_logp, bins=bins, density=True, alpha=0.55, label="Flow-val protons")
plt.hist(kaon_logp, bins=bins, density=True, alpha=0.45, label="Kaon candidates")
plt.xlabel("log p(z | L)")
plt.ylabel("Density")
plt.title("Log p(z | L) for proton train vs val vs kaons")
plt.xlim(-25, 2.5)
plt.grid(alpha=0.25)
plt.legend()
plt.tight_layout()
plt.show()
```

If you want the later notebook plot that colors kaon UMAP points by `kaon_logp`, it plugs directly into the optional `Z_umap` / `E_d` section above:

```python
values = np.asarray(kaon_logp, dtype=np.float32).reshape(-1)
values = np.maximum(values, -25)

plt.figure(figsize=(8, 6))
sc = plt.scatter(E_d[:, 0], E_d[:, 1], c=values, cmap="viridis", s=10, alpha=0.8, vmin=-25)
plt.colorbar(sc, label=r"log p(z \mid c(L)) (capped at -25)")
plt.title("Kaon candidates — Colored by kaon log-probability")
plt.xlabel("UMAP dim 1")
plt.ylabel("UMAP dim 2")
plt.tight_layout()
plt.show()
```

## 7. Turn flow scores into a classifier and inspect kaons with chi2 overlays

This uses a ROC curve on proton versus kaon negative log-likelihoods, chooses the threshold with Youden's J statistic, and then inspects kaon images above and below that threshold. The retained display function is the final one from the notebook, the version that includes both kaon-hypothesis and proton-hypothesis chi2 values.

```python
proton_logp_all = np.concatenate([train_logp, val_logp])

y_true = np.concatenate([
    np.zeros(len(proton_logp_all), dtype=np.int32),
    np.ones(len(kaon_logp), dtype=np.int32),
])

nll_all = -np.concatenate([proton_logp_all, kaon_logp])

fpr, tpr, thr_nll = roc_curve(y_true, nll_all, pos_label=1)
J = tpr - fpr
best_idx = np.argmax(J[1:]) + 1

best_thr_nll = thr_nll[best_idx]
best_thr_logp = -best_thr_nll

y_pred = (nll_all >= best_thr_nll).astype(np.int32)

tn, fp, fn, tp = confusion_matrix(y_true, y_pred).ravel()
precision, recall, f1, _ = precision_recall_fscore_support(
    y_true, y_pred, average="binary", zero_division=0
)
auc = roc_auc_score(y_true, nll_all)

print(f"Best threshold on log p(z|c(L)): {best_thr_logp:.6f}")
print(f"Equivalent threshold on NLL:      {best_thr_nll:.6f}")
print(f"TPR (recall): {recall:.4f}")
print(f"FPR:          {fp / (fp + tn):.4f}")
print(f"Precision:    {precision:.4f}")
print(f"F1:           {f1:.4f}")
print(f"ROC AUC:      {auc:.4f}")
print(f"Confusion matrix [[TN, FP], [FN, TP]] = [[{tn}, {fp}], [{fn}, {tp}]]")

nll_kaon = -kaon_logp
idx_above = np.where(nll_kaon >= best_thr_nll)[0]
idx_below = np.where(nll_kaon < best_thr_nll)[0]
top_idx = np.argsort(nll_kaon)[::-1][:16]

print(f"Kaons above threshold: {len(idx_above)} / {len(nll_kaon)}")
print(f"Kaons below threshold: {len(idx_below)} / {len(nll_kaon)}")

plt.figure(figsize=(8, 4))
plt.hist(nll_kaon, bins=7000, alpha=0.7, label="Kaons")
plt.axvline(best_thr_nll, color="red", ls="--", lw=2, label=f"best thr = {best_thr_nll:.3f}")
plt.xlabel("NLL = -log p(z|c(L))")
plt.ylabel("Count")
plt.title("Kaon NLL with decision threshold")
plt.xlim(-2.5, 30)
plt.legend()
plt.tight_layout()
plt.show()

k_np = k.cpu().numpy() if torch.is_tensor(k) else np.asarray(k)
kaons = kaons.reset_index(drop=True)
chi2_k = kaons["chi_squared_kaon"].to_numpy()
chi2_p = kaons["chi_squared_proton"].to_numpy()


def show_kaon_grid(indices, title, n_show=16, channel=0, seed=42):
    if len(indices) == 0:
        print(f"No samples for: {title}")
        return

    rng = np.random.default_rng(seed)
    pick = indices if len(indices) <= n_show else rng.choice(indices, size=n_show, replace=False)

    ncols = 4
    nrows = int(np.ceil(len(pick) / ncols))
    fig, axes = plt.subplots(nrows, ncols, figsize=(3 * ncols, 3 * nrows))
    axes = np.atleast_1d(axes).ravel()

    for ax, idx in zip(axes, pick):
        ax.imshow(k_np[idx, channel], cmap="gray", aspect="auto")
        ax.set_title(
            f"idx={idx}\n"
            f"NLL={nll_kaon[idx]:.2f}\n"
            f"$\\chi^2_K$={chi2_k[idx]:.2f}  $\\chi^2_P$={chi2_p[idx]:.2f}",
            fontsize=9,
        )
        ax.axis("off")

    for ax in axes[len(pick):]:
        ax.axis("off")

    fig.suptitle(title, y=1.02)
    plt.tight_layout()
    plt.show()


show_kaon_grid(idx_above, "Kaons classified as kaon (NLL >= threshold)", n_show=16, channel=0)
show_kaon_grid(idx_below, "Kaons missed by threshold (NLL < threshold)", n_show=16, channel=0)
show_kaon_grid(top_idx, "Top anomalous kaons (highest NLL)", n_show=16, channel=0)

fig, ax = plt.subplots(1, 2, figsize=(12, 4), sharey=True)

ax[0].hist(chi2_k[idx_above], bins="auto", alpha=0.6, density=True, label="NLL >= thr")
ax[0].hist(chi2_k[idx_below], bins="auto", alpha=0.6, density=True, label="NLL < thr")
ax[0].set_xlabel(r"$\chi^2$ (kaon hypothesis)")
ax[0].set_ylabel("Density")
ax[0].set_title(r"Kaon $\chi^2_K$")
ax[0].legend()

ax[1].hist(chi2_p[idx_above], bins="auto", alpha=0.6, density=True, label="NLL >= thr")
ax[1].hist(chi2_p[idx_below], bins="auto", alpha=0.6, density=True, label="NLL < thr")
ax[1].set_xlabel(r"$\chi^2$ (proton hypothesis)")
ax[1].set_title(r"Kaon $\chi^2_P$")
ax[1].legend()

plt.tight_layout()
plt.show()

plt.figure(figsize=(6, 5))
sc = plt.scatter(chi2_p, chi2_k, c=nll_kaon, s=12, alpha=0.6, cmap="viridis")
plt.colorbar(sc, label="NLL = -log p(z|c(L))")
plt.xlabel(r"$\chi^2$ (proton hypothesis)")
plt.ylabel(r"$\chi^2$ (kaon hypothesis)")
plt.title(r"Kaons: $\chi^2_P$ vs $\chi^2_K$")
plt.tight_layout()
plt.show()
```

## 8. Keep the downstream diagnostics that still use the current flow scores

Everything below still belongs to the same affine-flow analysis. These cells do not introduce a new flow model; they just slice and summarize the current flow outputs.

### 8.1 Height and chi2 summaries above and below threshold

These cells ask whether the threshold split correlates with obvious track properties and chi2 behavior.

```python
kaons = kaons.reset_index(drop=True)

heights = kaons["height"].to_numpy(dtype=np.float32)
thr = best_thr_nll

mask_below = nll_kaon < thr
mask_above = nll_kaon >= thr

h_below = heights[mask_below]
h_above = heights[mask_above]

print(
    f"Below threshold: n={len(h_below)}, "
    f"mean={h_below.mean():.2f}, std={h_below.std(ddof=1):.2f}"
)
print(
    f"Above threshold: n={len(h_above)}, "
    f"mean={h_above.mean():.2f}, std={h_above.std(ddof=1):.2f}"
)

bins = np.linspace(heights.min(), heights.max(), 40)

plt.figure(figsize=(8, 5))
plt.hist(h_below, bins=bins, alpha=0.6, density=True, label="Below threshold")
plt.hist(h_above, bins=bins, alpha=0.6, density=True, label="Above threshold")
plt.xlabel("Height")
plt.ylabel("Density")
plt.title("Kaon height distribution by threshold")
plt.grid(alpha=0.3)
plt.legend()
plt.tight_layout()
plt.show()

chi2_k = kaons["chi_squared_kaon"].to_numpy(dtype=np.float32)
chi2_p = kaons["chi_squared_proton"].to_numpy(dtype=np.float32)

p_lt_k = chi2_p < chi2_k

pct_below = 100 * p_lt_k[mask_below].mean() if mask_below.any() else np.nan
pct_above = 100 * p_lt_k[mask_above].mean() if mask_above.any() else np.nan

print(
    f"Below threshold: {p_lt_k[mask_below].sum()}/{mask_below.sum()} = "
    f"{pct_below:.2f}% have chi2_p < chi2_k"
)
print(
    f"Above threshold: {p_lt_k[mask_above].sum()}/{mask_above.sum()} = "
    f"{pct_above:.2f}% have chi2_p < chi2_k"
)

for label, mask in [("Below threshold", mask_below), ("Above threshold", mask_above)]:
    chi2k_grp = chi2_k[mask]
    chi2p_grp = chi2_p[mask]

    print(
        f"{label}: "
        f"n={mask.sum()} | "
        f"chi2_k mean={chi2k_grp.mean():.3f}, std={chi2k_grp.std(ddof=1):.3f} | "
        f"chi2_p mean={chi2p_grp.mean():.3f}, std={chi2p_grp.std(ddof=1):.3f}"
    )
```

### 8.2 Mass and reconstruction-error splits for kaons

This is the notebook's "checking on mass" block. It keeps the current classifier fixed and asks how beamline mass and VAE reconstruction error behave above and below the chosen threshold.

```python
beam_mass = kaons["beamline_mass"].to_numpy(dtype=np.float32)

assert len(beam_mass) == len(nll_kaon)

mass_below = beam_mass[mask_below]
mass_above = beam_mass[mask_above]

print(
    f"Below threshold: n={len(mass_below)}, "
    f"mean={mass_below.mean():.2f}, std={mass_below.std():.2f}"
)
print(
    f"Above threshold: n={len(mass_above)}, "
    f"mean={mass_above.mean():.2f}, std={mass_above.std():.2f}"
)

plt.figure(figsize=(7, 5))
plt.boxplot(
    [mass_below, mass_above],
    labels=["Below threshold", "Above threshold"],
    showfliers=False,
)
plt.ylabel("Beamline mass [MeV]")
plt.title("Kaon beamline mass split by flow threshold")
plt.grid(alpha=0.3)
plt.tight_layout()
plt.show()

RE_kaon = np.asarray(RE_per_sample_k, dtype=np.float32).reshape(-1)

for label, mask in [("Below threshold", mask_below), ("Above threshold", mask_above)]:
    re_grp = RE_kaon[mask]
    print(
        f"{label}: "
        f"n={mask.sum()} | "
        f"RE mean={re_grp.mean():.6f}, std={re_grp.std(ddof=1):.6f}"
    )

re_below = RE_kaon[mask_below]
re_above = RE_kaon[mask_above]

plt.figure(figsize=(7, 5))
plt.boxplot(
    [re_below, re_above],
    labels=["Below threshold", "Above threshold"],
    showfliers=False,
)
plt.ylabel("Reconstruction Error")
plt.title("Kaon RE split by flow threshold")
plt.grid(alpha=0.3)
plt.tight_layout()
plt.show()
```

### 8.3 Validation-proton summary under the same threshold

The notebook also applies the kaon/proton threshold to the held-out proton validation set so the same split can be compared against beamline mass, chi2 values, and reconstruction error on known protons.

```python
val_protons = protons.iloc[val_idx].reset_index(drop=True)

nll_val = np.asarray(proton_score, dtype=np.float32).reshape(-1)
RE_val = np.asarray(RE_per_sample_val, dtype=np.float32).reshape(-1)

mass_val = val_protons["beamline_mass"].to_numpy(dtype=np.float32)
chi2k_val = val_protons["chi_squared_kaon"].to_numpy(dtype=np.float32)
chi2p_val = val_protons["chi_squared_proton"].to_numpy(dtype=np.float32)

assert (
    len(val_protons)
    == len(nll_val)
    == len(RE_val)
    == len(mass_val)
    == len(chi2k_val)
    == len(chi2p_val)
)

mask_val_below = nll_val < thr
mask_val_above = nll_val >= thr


def mean_std(x):
    x = np.asarray(x, dtype=np.float32)
    return x.mean(), x.std(ddof=1)


rows = []
for label, mask in [("Below threshold", mask_val_below), ("Above threshold", mask_val_above)]:
    m_mean, m_std = mean_std(mass_val[mask])
    k_mean, k_std = mean_std(chi2k_val[mask])
    p_mean, p_std = mean_std(chi2p_val[mask])
    r_mean, r_std = mean_std(RE_val[mask])

    rows.append(
        {
            "group": label,
            "n": int(mask.sum()),
            "mass_mean": m_mean,
            "mass_std": m_std,
            "chi2_k_mean": k_mean,
            "chi2_k_std": k_std,
            "chi2_p_mean": p_mean,
            "chi2_p_std": p_std,
            "RE_mean": r_mean,
            "RE_std": r_std,
        }
    )

summary = pd.DataFrame(rows)

for _, row in summary.iterrows():
    print(
        f"{row['group']}: n={row['n']} | "
        f"mass={row['mass_mean']:.2f} ± {row['mass_std']:.2f} | "
        f"chi2_k={row['chi2_k_mean']:.3f} ± {row['chi2_k_std']:.3f} | "
        f"chi2_p={row['chi2_p_mean']:.3f} ± {row['chi2_p_std']:.3f} | "
        f"RE={row['RE_mean']:.6f} ± {row['RE_std']:.6f}"
    )

print(
    f"{summary['n'].iloc[1] / (summary['n'].iloc[0] + summary['n'].iloc[1]) * 100:.2f} % "
    "are above threshold"
)

summary
```

### 8.4 Build the selected-kaon event list

After thresholding, the notebook turns the selected kaon indices into a run/subrun/event table. This is the point where the current affine-flow pipeline produces a concrete candidate list.

```python
proton_nll = np.asarray(proton_score, dtype=np.float32).reshape(-1)
kaon_nll = np.asarray(kaon_score, dtype=np.float32).reshape(-1)

y_true = np.r_[
    np.zeros(len(proton_nll), dtype=np.int32),
    np.ones(len(kaon_nll), dtype=np.int32),
]
nll_all = np.r_[proton_nll, kaon_nll]

fpr, tpr, thr = roc_curve(y_true, nll_all, pos_label=1)
best_idx = np.argmax((tpr - fpr)[1:]) + 1
best_thr_nll = float(thr[best_idx])

idx_selected = np.where(kaon_nll >= best_thr_nll)[0]

selected_kaon_rows = (
    kaons.loc[
        idx_selected,
        [
            "run",
            "subrun",
            "event",
            "height",
            "beamline_mass",
            "chi_squared_kaon",
            "chi_squared_proton",
        ],
    ]
    .assign(kaon_idx=idx_selected, nll=kaon_nll[idx_selected])
    .reset_index(drop=True)
)

selected_events = (
    selected_kaon_rows[["run", "subrun", "event"]]
    .drop_duplicates()
    .sort_values(["run", "subrun", "event"])
    .reset_index(drop=True)
)

# selected_events.to_csv("selected_kaons.csv", index=False)
```

## Stop point

This is where the extraction intentionally stops.

Everything above still belongs to the current affine-flow analysis. What is not copied after this point is the later notebook branch that introduces new normalising-flow logic, including the later spline-flow experiments and other replacement flow cells.
