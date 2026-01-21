# 📸 TSS Photo Manager

**TSS Photo Manager** is a desktop application built with **C++** and the **Qt Framework** for **Windows**. It is designed to provide users with a fast and intuitive way to browse, organize, and perform essential edits on their photo collections.

## 🚀 Key Features

### Photo Management
* **Easy Import:** Import photos directly from local folders or external drives.
* **Smart Cataloging:** Add tags, ratings, and comments to keep your library organized.
* **Advanced Filtering:** Search and filter your collection by date, tag, or rating.
* **Sorting:** Organize your view by name, date, or rating.

### Photo Editing
* **Essential Tools:** Crop, rotate, and adjust brightness, contrast, and saturation.
* **Watermarking:** Apply custom watermarks to protect your work.
* **Live Preview:** View changes in real-time before applying them to the file.

### Import & Export
* **Format Support:** Export photos to `.jpg`, `.png`, `.gif`, and `.tiff` formats.
* **Target Control:** Choose specific destination folders for your exported files.

---

## 🏗️ System Architecture
The application follows a clean, layered architecture to ensure maintainability and performance:

1.  **UI Layer (TSS_App):** Handles user interaction and data display via Qt Widgets.
2.  **Model Layer (PhotoTableModel):** Manages application logic, including sorting, filtering, and pagination.
3.  **Data Layer (PhotoMetadataManager):** Handles persistent metadata storage using a local JSON-based system.

---

## ⚙️ Technical Specifications & NFRs
* **Language/Framework:** C++ / Qt (Open-source version).
* **Performance:** Loads 10,000 photos within 15 seconds.
* **Efficiency:** Designed for low memory usage through lazy loading and pagination.
* **Usability:** Basic editing operations are achievable within a maximum of 3 clicks.
* **Privacy:** Works fully offline; all metadata and photos are stored locally.
* **Constraints:** Supports a maximum file size of 10 MB per photo.

---

## 🛠️ System Requirements
* **Operating System:** Windows.
* **Hardware:** Minimum 8 GB RAM recommended for optimal performance with large catalogs.
* **Installation:** Standalone executable, no complex installation required.

---

## 📝 Project Information
* **Author:** Júlia Ondrušová.
* **Date:** October 2025 - Januar 2026.

---
*This project was developed as part of a software engineering assignment focusing on architectural design and functional requirement mapping.*
