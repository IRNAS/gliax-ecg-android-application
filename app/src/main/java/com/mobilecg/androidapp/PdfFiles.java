package com.mobilecg.androidapp;

import java.util.Date;

public class PdfFiles implements Comparable<PdfFiles> {
    private String fileName;
    private Date fileDate;

    public PdfFiles(String name, long lastModified) {
        fileName = name;
        fileDate = new Date(lastModified);
}

    public String getFileName() {
        return fileName;
    }

    public int compareTo(PdfFiles other) {
        return fileDate.compareTo(other.fileDate);
    }
}
