/*
 * This file is part of gliax-ecg-android-application
 * Glia is a project with the goal of releasing high quality free/open medical hardware
 * to increase availability to those who need it.
 * For more information visit Glia Free Medical hardware webpage: https://glia.org/
 *
 * Made by Institute Irnas (https://www.irnas.eu/)
 * Copyright (C) 2019 Vid Rajtmajer
 *
 * Based on MobilECG, an open source clinical grade Holter ECG.
 * For more information visit http://mobilecg.hu
 * Authors: Robert Csordas, Peter Isza
 *
 * This project uses modified version of usb-serial-for-android driver library
 * to communicate with Irnas made ECG board.
 * Original source code: https://github.com/mik3y/usb-serial-for-android
 * Library made by mik3y and kai-morich, modified by Vid Rajtmajer
 * Licensed under LGPL Version 2.1
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package com.mobilecg.androidapp;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Locale;

public class MyListViewAdapter extends BaseAdapter {

    Context myContext;
    LayoutInflater inflater;
    private ArrayList<PdfFiles> arrayList;

    public MyListViewAdapter(Context context) {
        myContext = context;
        inflater = LayoutInflater.from(myContext);
        arrayList = new ArrayList<>();
        arrayList.addAll(EcgActivity.namesOfFiles);
    }

    public class ViewHolder {
        TextView name;
    }

    @Override
    public int getCount() {
        return EcgActivity.namesOfFiles.size();
    }

    @Override
    public Object getItem(int position) {
        return EcgActivity.namesOfFiles.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View view, ViewGroup parent) {
        final ViewHolder holder;
        if (view == null) {
            holder = new ViewHolder();
            view = inflater.inflate(R.layout.listview_item, null);
            // Locate the TextViews in listview_item.xml
            holder.name = (TextView) view.findViewById(R.id.name);
            view.setTag(holder);
        } else {
            holder = (ViewHolder) view.getTag();
        }
        // Set the results into TextViews
        holder.name.setText(EcgActivity.namesOfFiles.get(position).getFileName());
        return view;
    }

    public void filter(String charText) {
        charText = charText.toLowerCase(Locale.getDefault());
        EcgActivity.namesOfFiles.clear();
        if (charText.length() == 0) {
            EcgActivity.namesOfFiles.addAll(arrayList);
        } else {
            for (PdfFiles pf : arrayList) {
                if (pf.getFileName().toLowerCase(Locale.getDefault()).contains(charText)) {
                    EcgActivity.namesOfFiles.add(pf);
                }
            }
        }
        notifyDataSetChanged();
    }
}
