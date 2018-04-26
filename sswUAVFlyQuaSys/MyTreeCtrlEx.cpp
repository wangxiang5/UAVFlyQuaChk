// MyTreeCtrlEx.cpp : 实现文件
//

#include "stdafx.h"
#include "sswUAVFlyQuaSys.h"
#include "MainFrm.h"
#include "sswUAVFlyQuaSysDoc.h"
#include "sswUAVFlyQuaSysView.h"
#include "MyTreeCtrlEx.h"
#include "MyFunctions.h"
// CMyTreeCtrlEx

IMPLEMENT_DYNAMIC(CMyTreeCtrlEx, CTreeCtrl)

CMyTreeCtrlEx::CMyTreeCtrlEx()
{

}

CMyTreeCtrlEx::~CMyTreeCtrlEx()
{
}


BEGIN_MESSAGE_MAP(CMyTreeCtrlEx, CTreeCtrl)
	ON_NOTIFY_REFLECT(NM_CLICK, &CMyTreeCtrlEx::OnNMClick)
END_MESSAGE_MAP()



// CMyTreeCtrlEx 消息处理程序




void CMyTreeCtrlEx::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);//转化为客户坐标
	UINT uFlags;
	HTREEITEM CurrentItem;

	CurrentItem = HitTest(point, &uFlags);//获得当前点击节点的ITEM

	if(CurrentItem)
	{
		int nData = GetItemData(CurrentItem);
		
		if(nData>=0)  //无效影像
		{
			vector<CString> vecItemInfo;
			if(nData==0)
				for (int i = 0; i<GetDocHand()->m_nImgNum; i++)
				{
					if(GetDocHand()->m_vecImgInfo[i].m_bValid==false)
					{
						vecItemInfo.push_back(FunGetFileName(GetDocHand()->m_vecImgInfo[i].m_strImgPath,true));
					}
				}
			else 
			{
				vector<int> vImgIdx = GetDocHand()->m_FlyQuaPrj.GetStripImgIdx()[nData-1];
				for (int i = 0; i<vImgIdx.size(); i++)
				{
					vecItemInfo.push_back(FunGetFileName(GetDocHand()->m_vecImgInfo[vImgIdx[i]].m_strImgPath,true));
				}
				vector<int>().swap(vImgIdx);
			}
		//	GetMainFramHand()->m_wndStripImg.LoadData(vecItemInfo);
			GetMainFramHand()->ShowStripList(vecItemInfo);
			vector<CString>().swap(vecItemInfo);
		}
		GetViewHand()->m_nStripIdxSelected = nData-1;
		GetViewHand()->ShowSeletctedStrip();
		GetViewHand()->Invalidate();
	}
	*pResult = 0;
}
