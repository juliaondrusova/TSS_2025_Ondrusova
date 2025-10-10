# URS-lite — Photo Software

## 1. Vision
Create a simple and fast Windows application (in C++ and Qt) that allows users to organize and perform basic edits on photos, browse them, search, and export to various formats.

---

## 2. Stakeholders (detect at least 3 stakeholders)
| Stakeholder | Needs / Goals | Influence |
|-------------|---------------|-----------|
| Michal Kollár | Requires a functional application by the end of January, with continuous progress updates | High |
| End User | Wants to easily manage and edit their photos | High |
| University / Faculty | Expects the application to be original (not a plagiarism) | Medium |
| Future Employer | Wants the graduate to have practical experience in software engineering | Medium |

---

## 3. User Types / Personas
- **Persona 1:** Enthusiastic Photographer — has many RAW and TIFF photos, needs more advanced edits (filter application, various watermarks) and tag-based organization.  
- **Persona 2:** Regular User — has photos from a phone in one format, wants to quickly browse, sort, and occasionally edit them.  

---

## 4. User Stories (Backlog) with MoSCoW

- [M] The user must be able to crop, rotate, and adjust brightness, contrast, and saturation of photos.  
- [M] The application must support the following image file types: .jpg, .png, .HEIC.  
- [M] Tags, ratings, and comments must be assignable to photos.  
- [M] The application must allow searching photos by date, tag, and color.  
- [M] Watermarks with a logo must be applicable to photos.  
- [M] It must be possible to import photos from an external drive and export them to other image file types.  
- [M] Photos must be processable both in bulk and individually.  
- [S] The application should support the .RAW image file type.  
- [S] The application should have a simple, clean, but modern user interface.  
- [C] The application could have a dark mode.  
- [C] The application could support the .tiff image file type.  
- [C] The application could include preset filters for quick photo enhancement.  
- [W] Category suggestions will be generated automatically (using AI) and can later be modified.  

---

## 5. Acceptance Criteria (for top Must stories)

**Story:** As a user, I want to search photos by date, tag, or color.

- AC-1: The user enters a date/tag/color, and the system returns the matching photos.  
- AC-2: The search completes within 5 seconds even for 10,000 photos.  
- AC-3: The user can combine multiple filters.  

---

## 6. BDD Scenario for 2 Stories (Given-When-Then)

**Story:** The user wants to crop, rotate, and adjust brightness, contrast, and saturation.

Given: the user has an open photo.  
When: they use the crop, rotate, or brightness/contrast/saturation adjustment function.  
Then: the photo is modified, and a preview of the changes is displayed.  

**Story:** The user wants to import and export photos.

Given: the user selects a folder or an external drive.  
When: they click “Import” or “Export.”  
Then: the photos are imported into the application or exported to the target folder in the selected format.  

---

## 7. Non-functional Requirements (NFRs) (4–6 metrics)

- **Performance:** 10,000 photos load within 5 seconds.  
- **Reliability:** In case of a crash, the application automatically saves ongoing work.  
- **Usability:** Photos can be browsed quickly and easily.  
- **UI:** Must be simple, modern, and optionally support Dark Mode.  
- **Availability:** The program must work without complex installation and must not be memory-intensive.  

---

## 8. Out of Scope (Won’t have this time)
- AI-based categorization  
- Mobile application  
- Cloud backup of all photos  

---

## 9. Open Questions and Assumptions
- **Q1:** What exact export formats will be mandatory?  
- **Q2:** Within what time limit must 10,000 photos be loaded?  
- **Q3:** What is the maximum allowed size of the resulting application?  
- **Q4:** More detailed specification of batch processing.  
- **Assumption 1:** Users will work only on Windows systems.  
- **Assumption 2:** Only freely licensed libraries will be used.  

---
