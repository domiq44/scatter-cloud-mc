const cards = Array.from(document.querySelectorAll('.card'));
const angleSelect = document.getElementById('angleFilter');
const tauSelect = document.getElementById('tauFilter');
const scaleToggle = document.getElementById('scaleFilter');
const cosAlphaDToggle = document.getElementById('cosAlphaDFilter');

cards.sort((a, b) => Number(a.dataset.tau) - Number(b.dataset.tau));
const grid = document.getElementById('grid');
cards.forEach(card => grid.appendChild(card));

const angles = [...new Set(cards.map(c => c.dataset.angle))].sort((a,b)=>a-b);
const taus   = [...new Set(cards.map(c => c.dataset.tau))].sort((a,b)=>a-b);

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

function applyFilter() {
  const a = angleSelect.value;
  const t = tauSelect.value;
  for (const c of cards) {
    const okA = (a === 'all' || c.dataset.angle === a);
    const okT = (t === 'all' || c.dataset.tau === t);
    c.classList.toggle('hidden', !(okA && okT));
  }
}

angleSelect.addEventListener('change', applyFilter);
tauSelect.addEventListener('change', applyFilter);
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
