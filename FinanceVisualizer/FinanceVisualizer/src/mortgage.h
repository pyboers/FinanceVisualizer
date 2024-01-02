#pragma once
#include "imgui/imgui.h"
#include "implot/implot.h"
#include <vector>
#include <algorithm>
#include <numeric>
#include <format>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <locale>


namespace mortgage {

	static std::vector<float> netInterestYs;
	static std::vector<float> owingYs;
	static std::vector<float> cumulativeSavedYs;
	static std::vector<float> cumulativeLostYs;
	


	static std::vector<float> xs;

	static double rent = 1045;
	static double valuation = 400000;
	static double bringIn = 0;
	static double borrow;
	static double interestRate = 0.0416;
	static double nhgRate = 0.006;
	static double woz;
	static int repaymentFrequency = 12;
	static double repaymentLength = 30;
	static double interestDeductRate = 0.37;

	static double overPayment = 0;
	
	static float annuityFactor;
	static float annuityPayment;

	static std::string formatCurrency(double value) {
		std::ostringstream oss;
		std::locale loc("");
		oss.imbue(loc);
		oss << std::showbase << std::put_money(value);

		auto str = oss.str();
		return oss.str();
	}

	static void init() {
		woz = 0.0045 * valuation;
		borrow = valuation - bringIn;
		double interestFactor = interestRate / repaymentFrequency;
		double compound = pow(1 + interestFactor, repaymentFrequency * repaymentLength);
		annuityFactor = (compound - 1) / (interestFactor * compound);
		annuityPayment = ceil(borrow / annuityFactor);
		xs.resize(repaymentLength * repaymentFrequency);
		netInterestYs.resize(xs.size());
		owingYs.resize(xs.size());
		cumulativeSavedYs.resize(xs.size());
		cumulativeLostYs.resize(xs.size());
		std::iota(xs.begin(), xs.end(), 1);
	}

	static void render() {
		woz = 0.0045 * valuation;
		borrow = valuation - bringIn;
		double interestFactor = interestRate / repaymentFrequency;
		double compound = pow(1 + interestFactor, repaymentFrequency * repaymentLength);
		annuityFactor = (compound - 1) / (interestFactor * compound);
		annuityPayment = ceil((borrow / annuityFactor));
		xs.resize(repaymentLength * repaymentFrequency);
		netInterestYs.resize(xs.size());
		owingYs.resize(xs.size());
		cumulativeSavedYs.resize(xs.size());
		cumulativeLostYs.resize(xs.size());
		std::iota(xs.begin(), xs.end(), 1);

		double owing = borrow;
		double cumulativeSaved = 0;
		double cumulativeLost = 0;
		for (int i = 0; i < repaymentLength * repaymentFrequency; i++) {
			double interest = ceil(owing * (interestRate / repaymentFrequency));
			double principal = annuityPayment - interest;
			double taxReturn = floor(((woz / repaymentFrequency) - interest) * interestDeductRate);
			double netInterest = interest + taxReturn;
			netInterestYs[i] = netInterest;

			cumulativeSaved += rent - netInterest;
			cumulativeSavedYs[i] = cumulativeSaved;



			owing -= principal;
			owingYs[i] = owing;

			cumulativeLost += netInterest;
			cumulativeLostYs[i] = cumulativeLost;

		}
		ImGui::Begin("Annuity Mortgage");
		ImPlot::BeginPlot("Graph", ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()));
		bool hovered = false;
		bool held = false;
		ImPlot::DragLineY(0, &valuation, ImVec4(1, 0, 0, 1), 1.5f, ImPlotDragToolFlags_Delayed, nullptr, &hovered, &held);
		valuation = max(0, round(valuation));
		if (hovered || held) {
			ImGui::BeginTooltip();
			ImGui::Text(std::format("Valuation = {}", formatCurrency(valuation)).c_str());
			ImGui::EndTooltip();
		}

		ImPlot::DragLineY(0, &bringIn, ImVec4(0, 1, 0, 1), 1.5f, ImPlotDragToolFlags_Delayed, nullptr, &hovered, &held);
		bringIn = max(0, min(round(bringIn), valuation));
		if (hovered || held) {
			ImGui::BeginTooltip();
			ImGui::Text(std::format("Bring In = {}", formatCurrency(bringIn)).c_str());
			ImGui::EndTooltip();
		}

		repaymentLength *= repaymentFrequency;
		ImPlot::DragLineX(0, &repaymentLength, ImVec4(1, 1, 1, 1), 1.5f, ImPlotDragToolFlags_Delayed, nullptr, &hovered, &held);
		repaymentLength = max(0, min(30, round(repaymentLength / repaymentFrequency)));
		if (hovered || held) {
			ImGui::BeginTooltip();
			ImGui::Text(std::format("Mortgage Period = {} years", repaymentLength).c_str());
			ImGui::EndTooltip();
		}


		ImPlot::PlotLine("Owing", xs.data(), owingYs.data(), xs.size(), ImPlotLineFlags_None, 0);
		ImPlot::PlotLine("Net Interest", xs.data(), netInterestYs.data(), xs.size(), ImPlotLineFlags_None, 0);
		ImPlot::PlotLine("Cumulative Saved", xs.data(), cumulativeSavedYs.data(), xs.size(), ImPlotLineFlags_None, 0);
		ImPlot::PlotLine("Cumulative Lost", xs.data(), cumulativeLostYs.data(), xs.size(), ImPlotLineFlags_None, 0);
		ImPlotPoint mPos = ImPlot::GetPlotMousePos();
		if (mPos.x >= 1 && mPos.x < xs.size()) {
			ImPlot::Annotation((int)mPos.x, owingYs[(int)mPos.x - 1], ImVec4(1, 1, 1, 1), ImVec2(20, 0), true, true);
			ImPlot::Annotation((int)mPos.x, netInterestYs[(int)mPos.x - 1], ImVec4(1, 1, 1, 1), ImVec2(20, 0), true, true);
			ImPlot::Annotation((int)mPos.x, cumulativeLostYs[(int)mPos.x - 1], ImVec4(1, 1, 1, 1), ImVec2(20, 0), true, true);
			ImPlot::Annotation((int)mPos.x, cumulativeSavedYs[(int)mPos.x - 1], ImVec4(1, 1, 1, 1), ImVec2(20, 0), true, true);
		}
		ImPlot::EndPlot();


		ImGui::End();
	}
}
