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
