const cards = Array.from(document.querySelectorAll('.card'));
const geometrySelect = document.getElementById('geometryFilter');
const angleSelect = document.getElementById('angleFilter');
const tauSelect = document.getElementById('tauFilter');
const photonSelect = document.getElementById('photonFilter');
const scaleToggle = document.getElementById('scaleFilter');
const cosAlphaDToggle = document.getElementById('cosAlphaDFilter');

cards.sort((a, b) => Number(a.dataset.tau) - Number(b.dataset.tau));
const grid = document.getElementById('grid');
cards.forEach(card => grid.appendChild(card));

const angles = [...new Set(cards.map(c => c.dataset.angle))].sort((a,b)=>a-b);
const taus   = [...new Set(cards.map(c => c.dataset.tau))].sort((a,b)=>a-b);
const photonCounts = [...new Set(cards.map(c => c.dataset.photons).filter(Boolean))].sort((a,b)=>Number(a)-Number(b));

for (const a of angles) {
  const option = document.createElement('option');
  option.value = a;
  option.textContent = `${a}°`;
  angleSelect.append(option);
}
for (const t of taus) {
  const option = document.createElement('option');
  option.value = t;
  option.textContent = t;
  tauSelect.append(option);
}
for (const count of photonCounts) {
  const option = document.createElement('option');
  option.value = count;
  option.textContent = Number(count).toLocaleString('fr-FR');
  photonSelect.append(option);
}

function applyFilter() {
  const geometry = geometrySelect ? geometrySelect.value : 'all';
  const a = angleSelect.value;
  const t = tauSelect.value;
  const photonCount = photonSelect ? photonSelect.value : 'all';
  for (const c of cards) {
    const cardGeometry = c.dataset.geometry || 'sphere';
    const okGeometry = (geometry === 'all' || cardGeometry === geometry);
    const okA = (a === 'all' || c.dataset.angle === a);
    const okT = (t === 'all' || c.dataset.tau === t);
    const okPhotons = photonCount === 'all' || c.dataset.photons === photonCount;
    c.classList.toggle('hidden', !(okGeometry && okA && okT && okPhotons));
  }
  for (const figure of document.querySelectorAll('.reflectance figure')) {
    const okGeometry = geometry === 'all' || figure.dataset.geometry === geometry;
    figure.classList.toggle('hidden', !okGeometry);
  }
}

if (geometrySelect) geometrySelect.addEventListener('change', applyFilter);
angleSelect.addEventListener('change', applyFilter);
tauSelect.addEventListener('change', applyFilter);
if (photonSelect) photonSelect.addEventListener('change', applyFilter);
applyFilter();

function applyScale() {
  if (!scaleToggle) return;

  const axisKey = cosAlphaDToggle && cosAlphaDToggle.checked ? 'mu' : 'alphaD';
  const scaleKey = scaleToggle.checked ? 'Log' : 'Linear';
  const sourceKey = `${axisKey}${scaleKey}Src`;
  for (const card of cards) {
    const image = card.querySelector('img');
    const source = card.dataset[sourceKey] || card.dataset.logSrc;
    if (!source) continue;
    image.src = source;
    const path = card.querySelector('code');
    if (path) path.textContent = source;
  }
}

if (scaleToggle) {
  scaleToggle.addEventListener('change', applyScale);
  applyScale();
}
if (cosAlphaDToggle) cosAlphaDToggle.addEventListener('change', applyScale);

// Lightbox
const lightbox = document.getElementById('lightbox');
const lightboxImg = document.getElementById('lightbox-img');

cards.forEach(card => {
  const img = card.querySelector('img');
  img.addEventListener('click', () => {
    lightboxImg.src = img.src;
    lightbox.style.display = "flex";
  });
});

lightbox.addEventListener('click', () => {
  lightbox.style.display = "none";
});
