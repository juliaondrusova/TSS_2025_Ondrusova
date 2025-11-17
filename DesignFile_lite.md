# Design File (lite)
**Project:** Photo Manager Application
**Author:** Júlia Ondrušová  
**Date:** 17.11  

This document describes the system design at both high level and low level.  
It connects the Software Requirements Specification (SRS) with the future implementation.  
The goal is to show how requirements are realized through components and classes.

---

## 1. System Overview
TSS Photo Manager is a Qt-based desktop application designed for importing, browsing, filtering, editing, and exporting photos.
The architecture follows a simple layered approach:

- **UI Layer (TSS_App + Qt Widgets):** handles user interaction and displays data.
- **Model Layer (PhotoTableModel + Photo):** manages photo data, filtering, paging, sorting, and editing.
- **Data Layer (PhotoMetadataManager + PhotoData):** responsible for persistent meta data storage using JSON.

Main system components:

- **TSS_App:** main window, user interaction hub.
- **PhotoTableModel:** central data model for displaying and manipulating photos.
- **PhotoMetadataManager:** singleton managing metadata persistence.
- **Photo:** data structure representing a single photo.
- **PhotoData:** – JSON-serializable structure storing user metadata.
---

## 2. Component View (High-level design)
This section explains how the main components of the system are separated and how they communicate.

Instead of drawing a UML diagram, describe the structure **in text form**.

### 2.1 Component Overview
The system is divided into three logical layers:
 
**UI Layer:**  
- Provides all visible controls (buttons, search fields, table view).
- Handles user commands such as import, export, filtering, and theme switching.
- Displays data provided by the model.

**Model Layer:**  
- Contains application logic for sorting, filtering, editing, and paging.
- Maintains in-memory collections of photos and filtered views.
- Synchronizes changes with the data layer.

**Data Layer:**  
- Stores and retrieves persistent metadata.
- Uses a JSON file to store tag, rating, and comment information.

### 2.2 Component Responsibilities

**TSS_App**  
- Responsibility: Main window, orchestrates user actions, connects menus/buttons with model operations.  
- Communicates with: PhotoTableModel (display & editing), PhotoExportDialog (export), FilterEngine (filtering).  
- Supports SRS: FR-1.1, FR-1.2, FR-1.3, FR-3.3, FR-3.4, FR-3.5  

**PhotoTableModel**  
- Responsibility: Manage photos in table format with pagination, sorting, filtering, and editing of tag/rating/comment.  
- Communicates with: TSS_App (UI), PhotoMetadataManager (load/save metadata).  
- Supports SRS: FR-1.2, FR-1.3, FR-1.4, FR-1.5, FR-2.3  

**PhotoMetadataManager**  
- Responsibility: Persistent storage of photo metadata, JSON serialization/deserialization, cleanup of non-existent files.  
- Communicates with: PhotoTableModel (for saving/loading metadata).  
- Supports SRS: FR-1.3, NFR-2, SC-6  

**Photo**  
- Responsibility: Represents a single photo and its attributes: file path, tag, rating, comment.  
- Communicates with: PhotoTableModel, PhotoMetadataManager.  
- Supports SRS: FR-1.3, FR-2.1, FR-2.2  

**FilterEngine**  
- Responsibility: Apply filters for date, tag, rating, and search queries.  
- Communicates with: PhotoTableModel to update active photo list.  
- Supports SRS: FR-1.4, FR-1.5  

**PhotoExportDialog**  
- Responsibility: Manage export of photos, browse target folder, validate paths, show progress.  
- Communicates with: TSS_App, PhotoMetadataManager.  
- Supports SRS: FR-3.4, FR-3.5  

---

## 3. Detailed Design of One Feature (Low-level design)

Feature: **Export Photos**

### 3.1 Class Diagram (UML)

Classes involved: `PhotoExportDialog`, `Photo`, `PhotoMetadataManager`, `PhotoTableModel`  

### 3.2 Class Descriptions
**PhotoExportDialog**  
- Attributes: m_photosToExport, m_tableWidget, m_progressBar  
- Methods: onBrowseClicked(), onExportClicked(), getSelectedPhotos(), validateAllPaths(), exportPhotos()  

**Photo**  
- Attributes: filePath, tag, rating, comment  
- Methods: toJson(), fromJson()  

**PhotoMetadataManager**  
- Attributes: m_metadata, m_currentFilePath  
- Methods: loadFromFile(), saveToFile(), getPhotoData(), setRating(), setTag(), setComment()  

**PhotoTableModel**  
- Attributes: m_allPhotos, m_filteredPhotos, m_pageSize, m_currentPage  
- Methods: addPhoto(), photoAt(), getPhotosMarkedForExport(), setDateFilter(), setTagFilter(), setRatingFilter(), applyFilters()  

---

## 4. Mapping Requirements → Design

| SRS Requirement ID | Short description of requirement                    | Implemented in (component / class / method)                  |
|--------------------|----------------------------------------------------|-------------------------------------------------------------|
| FR-1.1             | Import photos from folder or external device      | TSS_App::importPhotos(), PhotoTableModel::addPhoto()       |
| FR-1.2             | Display photos in gallery/list view               | PhotoTableModel, TSS_App (UI Table View)                   |
| FR-1.3             | Add tags, comments, rating                        | PhotoMetadataManager, PhotoTableModel::setData()           |
| FR-1.4             | Filter/search by date, tag, color                 | FilterEngine, PhotoTableModel::setDateFilter()/setTagFilter() |
| FR-1.5             | Sort photos by rating, date, name                 | PhotoTableModel::sort()                                     |
| FR-2.1             | Crop, rotate, adjust brightness/contrast/saturation | Photo (attributes updated via editing dialogs)            |
| FR-2.2             | Add watermark                                     | PhotoEditDialog                       |
| FR-2.3             | Bulk processing                                   | PhotoTableModel::getPhotosMarkedForExport(), PhotoExportDialog::exportPhotos() |
| FR-3.3             | Import from multiple devices                      | TSS_App::importPhotos()                                     |
| FR-3.4             | Export to other formats (.jpg, .png, .HEIC)      | PhotoExportDialog::exportPhotos()                          |
| FR-3.5             | Choose target folder for export                   | PhotoExportDialog::onBrowseClicked()                       |

---

## 5. Technical Constraints and Decisions

- C++ / Qt is used because the application must run on Windows and provide native UI.  
- Qt signals/slots implement the Observer pattern to notify the UI about data changes.  
- Metadata is stored in JSON and cached in memory to allow fast filtering and editing.  
- Table model (PhotoTableModel) supports pagination to limit memory usage when displaying large collections.  
- Export is managed via PhotoExportDialog with progress bar to satisfy SC-6.  

---

## 6. Open Risks / Questions

- Performance with very large libraries (>50,000 photos) – may need virtualized table view.   
- Determining whether edits overwrite original files or save new copies (Q1 from SRS).  

---

## 7. Summary
The application architecture separates UI, model, and data layers for maintainability.  
Components include TSS_App, PhotoTableModel, PhotoMetadataManager, FilterEngine, and PhotoExportDialog.  
The detailed design for the Export Photos feature ensures users can select, validate, and export photos with progress feedback.  
This design maps directly to all key functional requirements and meets performance, usability, and platform constraints from the SRS.

---