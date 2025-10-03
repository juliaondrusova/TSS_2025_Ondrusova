# URS-lite — Softvér na fotografie

## 1. Vision
Vytvoriť jednoduchú a rýchlu aplikáciu na Windows (v C++ a Qt), ktorá umožní organizáciu a základné úpravy fotografií, ich prezeranie, vyhľadávanie a export do rôznych formátov.

---

## 2. Stakeholders (detect at least 3 stakeholders)
| Stakeholder | Needs / Goals | Influence |
|-------------|---------------|-----------|
| Michal Kollár | Vyžaduje funkčnú aplikáciu do konca januára, s priebežným progresom | Vysoký |
| Koncový používateľ | Chce jednoducho spravovať a upravovať svoje fotografie | Vysoký |
| Univerzita / fakulta | Očakáva, že aplikácia bude originálna (nebude plagiátom) | Stredný |
| Budúci zamestnávateľ | Chce, aby absolvent mal praktické skúsenosti so softvérovým inžinierstvom. | Stredný |

---

## 3. User types / Personas
- **Persona 1:** Nadšený fotograf — má veľa RAW a TIFF fotiek, potrebuje aj zložitejšie úpravy (aplikácia filtrov, rôzne vodotlače) a organizáciu podľa tagov.  
- **Persona 2:** Bežný používateľ — má fotky z telefónu v jednom formáte, chce ich rýchlo prehliadať, triediť a občas upraviť. 

---

## 4. User Stories (Backlog) with MoSCoW

- [M] Používateľ musí byť schopný fotky orezávať, otáčať a upravovať im jas, kontrast a sýtosť.
- [M] Aplikácia musí podporovať nasledujúcce typy obrázkových súborov: .jpg, .png, .HEIC.
- [M] K fotkám musí byť možné pridať tag, hodnotenie a komentár.
- [M] V aplikácií musí byť možné vyhľadať fotky podľa dátumu, tagu a farby.
- [M] Na fotky musí byť možné pridať vodotlač s logom.
- [M] Je nutné, aby sa dali fotky importovať z externého disku a exportovať do iných typov obrázkových súborov.
- [M] Fotky sa musia dať spracovať hromadne, ale aj po jednom.
- [S] Aplikácia by mala podporovať typ obrázkoveho súboru .RAW.
- [S] Aplikácia by mala mať jendoduché, čisté, ale moderné užívateľské rozhranie.
- [S] Aplikácia by mohla mať dark mode.
- [C] Aplikácia by mohla podporovať typ obrázkoveho súboru .tiff.
- [C] V aplikácii by mohla byť možnosť použiť prednastavené filtre, aby sa dali fotky rýchlo vylepšiť.  
- [C] Navrhovanie kategórií bude automatické (pomocou AI) a bude ich potom možné upraviť. 

---

## 5. Acceptance Criteria (for top Must stories)

**Story:** Ako používateľ chcem vyhľadávať fotky podľa dátumu, tagu alebo farby.

- AC-1: Používateľ zadá dátum/tag/farbu a systém vráti zodpovedajúce fotky.  
- AC-2: Vyhľadávanie prebehne do 5 sekúnd aj pri 10 000 fotkách.  
- AC-3: Používateľ môže kombinovať viacero filtrov.
 
---

## 6. BDD Scenario for 2 stories (Given-When-Then)

**Story:** Používateľ chce orezávať, otáčať a upravovať jas, kontrast a sýtosť.

Given: používateľ má otvorenú fotografiu.
When: použije funkciu orezania, otočenia alebo zmeny jasu, kontrastu, sýtosti.
Then: fotografia sa upraví a zobrazí sa náhľad zmien.

**Story:** Používateľ chce fotky importovať a exportovať.

Given: používateľ zvolí priečinok alebo externý disk
When:  klikne na „Importovať“ alebo „Exportovať“.
Then: fotky sa importujú do aplikácie, respektíve exportujú do cieľového priečinka vo vybranom formáte.

---

## 7. Non-functional Requirements (NFRs) (4-6 metrics
- Performance: 10 000 fotiek sa načíta do 5 sekúnd.
- Reliability: Pri páde aplikácie sa automaticky uloží rozpracovaná práca.
- Usability: Fotky sa dajú rýchlo a jednoducho prehliadať.
- UI: Musí byť jednoduché, moderné, prípadne s možnosťou Dark Mode.
- Availability: Program musí fungovať bez potreby zložitej inštalácie a nesmie byť pamäťovo náročný.
---

## 8. Out of Scope (Won't have this time)
- Pokročilá AI kategorizácia.  
- Mobilná aplikácia
- Cloud backup všetkých fotiek  

---

## 9. Open Questions and Assumptions
- Q1: Aké presné exportné formáty budú povinné? 
- Q2: Do akého času je potrebné načíťať 10 000 fotiek?
- Q3: Akú maximálnu veľkosť môže mať výsledná aplikácia?
- Q4: Bližšia špecifikácia hromadného spracovania.
- Assumption 1: Používatelia budú pracovať iba na Windows systémoch.
- Assumption 2: Použijú sa len knižnice s voľnou licenciou.

---
