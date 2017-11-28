#include "../../Qing/qing_common.h"
#include "../../Qing/qing_dir.h"
#include "../../Qing/qing_file_reader.h"
#include "../../Qing/qing_file_writer.h"
#include "../../Qing/qing_string.h"


#define MAX_DISP 480.f
#define MIN_DISP 0.f

void qing_read_crop_infos(const string filename, vector<Point2i>& cxy, vector<Size>& csz, vector<int>& disp_ranges)
{
//    cout << filename << endl;
    fstream fin(filename.c_str(), ios::in);
    if(fin.is_open() == false)
    {
        cerr << "failed to open " << filename << endl;
        return ;
    }

    string s;
    vector<string> words(0);
    while(getline(fin, s))
    {
        if(s[0] == '#') continue;
        words.clear();
        qing_split_a_string_by_space(s, words);

        string camName = words[0];
        int cx = qing_string_2_int(words[1]);
        int cy = qing_string_2_int(words[2]);
        int w = qing_string_2_int(words[3]);
        int h = qing_string_2_int(words[4]);
        int d = qing_string_2_int(words[5]);

        cxy.push_back(Point2i(cx, cy));
        csz.push_back(Size(w, h));
        disp_ranges.push_back(d);
    }
//    cout << "haha" << endl;
}

void qing_read_disp_range_infos(const string filename, vector<float>& max_disps, vector<float>& min_disps) {
    fstream fin(filename.c_str(), ios::in);
    if(fin.is_open() == false) {
        cerr << "failed to open " << filename << endl;
        return ;
    }

    string s;
    vector<string> words(0);
    while(getline(fin, s)) {
        if('#'==s[0]) continue;
        words.clear();

        qing_split_a_string_by_space(s, words);
        string stereoName = words[0];
        int maxd = qing_string_2_int(words[1]);
        int mind = qing_string_2_int(words[2]);

        max_disps.push_back(maxd);
        min_disps.push_back(mind);
    }

    cout << "end of reading " << filename << ", max_disp size = " << max_disps.size() << ", min_disp size = " << min_disps.size() << endl;
}


string qing_get_msk_prefix(const string imageName)
{
    string tempstr = imageName.substr(0, imageName.rfind('_'));
    tempstr = tempstr.substr(0, tempstr.rfind('_')) + "_MSK";
    return tempstr ;

}


/* ./StereoInfos /media/ranqing/Work/RQProjects/ZJU/HumanDatas_20171018/Humans_frame/
 * /media/ranqing/Work/RQProjects/ZJU/HumanDatas_20171018/Calib_Results/
 *  /media/ranqing/Work/RQProjects/ZJU/HumanDatas_20171018/Infos_crop_points/ 0199 */
int main(int argc, char * argv[])
{
    //generate stereo infos frame by frame
    cout << "Usage : " << argv[0]
         << " /media/ranqing/Qing-WD-New/20171018/Rectified_Humans_frame/ "
         << " /media/ranqing/Qing-WD-New/20171018/Calib_Results/ "
         << " /media/ranqing/Qing-WD-New/20171018/Infos_crop_points/"
         << " 0001 " << endl;

    if (argc != 5) {
        cerr << "invalid arguments. " << endl;
        return -1;
    }

    string imageFolder = argv[1];         //image folder
    string calibFolder = argv[2];         //calib folder
    string infoFolder  = argv[3];         //cropper info folder
    string frameName = argv[4];           //frame name
    imageFolder = imageFolder + "FRM_"  + frameName ;

    cout << "frame: " << frameName << endl;              //FRM
    cout << "image folder: " << imageFolder << endl;
    cout << "calib folder: " << calibFolder << endl;
    cout << "info folder: " << infoFolder << endl;


    string cropInfos = infoFolder + "crop_infos_frm_" + frameName + ".txt";                 //media/ranqing/ranqing_wd/ZJU/HumanDatas/20161224/Humans_classified/ + crop_infos.txt
    string outfolder = "./FRM_" + frameName;
    qing_create_dir(outfolder);
    cout << "out folder: " << outfolder << endl;
    cout << "crop info name: " << cropInfos << endl;


    vector<string> imageLists(0);
    qing_get_all_files(imageFolder, imageLists);
   // cout << "hah" << endl;

    vector<Point2i> cxy(0);
    vector<Size> csz(0);
    vector<int> disp_ranges(0);
    qing_read_crop_infos(cropInfos, cxy, csz, disp_ranges);

    cout << imageLists.size() << " files . " << cxy.size() << '\t' << csz.size() << '\t' << disp_ranges.size() << endl;
    sort(imageLists.begin(), imageLists.end());


    //generate stereo infos: no N03~N06
    int cam_num = 64;
    string cams[64] = { "A01", "A02", "A03", "A04", "A05", "A06", "A07", "A08", "A09", "A10", "A11", "A12", "A13", "A14", "A15", "A16",
                        "B01", "B02", "B03", "B04", "B05", "B06", "B07", "B08", "B09", "B10", "B11", "B12", "B13", "B14", "B15", "B16",
                        "C01", "C02", "C03", "C04", "C05", "C06", "C07", "C08", "C09", "C10", "C11", "C12", "C13", "C14", "C15", "C16",
                        "L01", "L02", "L03", "L04", "L05", "L06", "N03", "N04", "N05", "N06", "R01", "R02", "R03", "R04", "R05", "R06",  };

    map<string, int> cameraNameDict;
    for(int i = 0; i < cam_num; ++i)
    {
        cameraNameDict.insert(pair<string,int>(cams[i], i));    //stereoname <-> stereoid
    }
    cout << "camera name dict done.. " << cameraNameDict.size() << " elements." << endl;
    map<string, string> stereoNameDict;
    for(int i = 0; i < cam_num; i+=2)
    {
        stereoNameDict.insert(pair<string,string>(cams[i], cams[i+1]));
    }
    cout << "stereo name dict done.. " << stereoNameDict.size() << " elements." << endl;

    vector<Mat> qmatrixs(0);
    for(int i = 0; i < cam_num; i+=2)
    {
        string stereoname = cams[i] + cams[i+1];
        string filename = "/stereo_" + stereoname + ".yml";
        string calibfile = calibFolder + filename;

        Mat Q;
        qing_read_stereo_yml_qmatrix(calibfile, Q);
        qmatrixs.push_back(Q);
        cout << filename << '\t' << qmatrixs[qmatrixs.size()-1] << endl;
    }

    string suffix = ".png";
    for(int i = 0; i < imageLists.size() - 1;)
    {
        string imgName0 = imageLists[i];
        string camName0 = imgName0.substr(0, 3);
        string imgName1 = imageLists[i+1];
        string camName1 = imgName1.substr(0, 3);

        string mskName0 = qing_get_msk_prefix(imgName0);                        //A03_IMG_1210
        string mskName1 = qing_get_msk_prefix(imgName1);                        //A04_IMG_1240

        mskName0 = mskName0 + "_" + frameName + suffix;
        mskName1 = mskName1 + "_" +  frameName + suffix;
        cout << mskName0 << endl;
        cout << mskName1 << endl;
        cout << endl;

        if(stereoNameDict.end() != stereoNameDict.find(camName0) && camName1 == stereoNameDict[camName0])
        {
            string stereoName = camName0 + camName1;
            string filename = outfolder + "/stereo_" + stereoName + ".info";

            i+=2;

            int idx0 = cameraNameDict[camName0];
            int idx1 = cameraNameDict[camName1];
            int stereoidx = idx0/2;

            cout << filename << endl;
            cout << stereoidx << ", " << stereoName << '\t' << cxy[idx0] << '\t' << cxy[idx1] << '\t' << csz[idx0] << '\t' << disp_ranges[idx0]<< endl;

            //stereoname
            //cam0
            //cam1
            //img0
            //img1
            //msk0
            //msk1
            //cropxy0
            //cropxy1
            //cropsz
            //MAXDISP
            //MINDISP
            //Qmatrix
            qing_write_stereo_info(filename, stereoidx, camName0, camName1, frameName, imgName0, imgName1, mskName0, mskName1, cxy[idx0], cxy[idx1], csz[idx0], disp_ranges[idx0], MIN_DISP, qmatrixs[stereoidx]);
         }
        else
        {
            cout << camName0 << ' ' << camName1 << endl;
            i++;
        }
    }
    return 1;
}
