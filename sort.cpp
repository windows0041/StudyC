#include <iostream>
#include <vector>
#include <ctime>
#include <chrono>
#include <string>
#include <random>
#include <algorithm>

using namespace std;

void ArrPrint(vector<int>& arr);
void QuickSortTool(vector<int>& arr, int left, int right);
void QuickSort(vector<int>& arr);
void BubbleSort(vector<int>& arr);
void ShellSort(vector<int>& arr);
void SelectSort(vector<int>& arr);
void MergeSort(vector<int>& arr);
void BucketSort(vector<int>& arr);
void CountingSort(vector<int>& arr);
void HeapSort(vector<int>& arr);

void ArrPrint(vector<int>& arr) {
    for (int i = 0; i < arr.size(); i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}

void QuickSortTool(vector<int>& arr, int left, int right) {
    if (left >= right) return;

    int pivot = arr[left];
    int i = left, j = right;

    while (i < j) {
        while (i < j && arr[j] >= pivot) {
            j--;
        }
        arr[i] = arr[j];
        while (i < j && arr[i] <= pivot) {
            i++;
        }
        arr[j] = arr[i];
    }
    arr[i] = pivot;

    QuickSortTool(arr, left, i - 1);
    QuickSortTool(arr, i + 1, right);
}

// Bubble Sort
void BubbleSort(vector<int>& arr) {
    for (int i = 0; i < arr.size() - 1; i++) {
        for (int j = 0; j < arr.size() - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                swap(arr[j], arr[j + 1]);
            }
        }

    }
}

// Quick Sort
void QuickSort(vector<int>& arr) {
    if (arr.size() > 0) {
        QuickSortTool(arr, 0, arr.size() - 1);
    }
}

// Insert Sort
void InsertionSort(vector<int>& arr) {
    for (int i = 1; i < arr.size(); i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

// Shell Sort
void ShellSort(vector<int>& arr) {
    for (int gpa = arr.size() / 2; gpa > 0; gpa /= 2) {
        for (int i = gpa; i < arr.size(); i++) {
            int temp = arr[i];
            int j;
            for (j = i; j >= gpa && arr[j - gpa] > temp; j -= gpa) {
                arr[j] = arr[j - gpa];
            }
            arr[j] = temp;
        }
    }
}

// Select Sort
void SelectSort(vector<int>& arr) {
    for (int i = 0; i < arr.size(); i++) {
        int min = arr[i], index = i, j = i;
        while (j < arr.size()) {
            if (arr[j] > min) {
                min = arr[j];
                index = j;
            }
            j++;
        }
        swap(arr[i], arr[index]);
    }
}

// Merge Sort
void MergeTool(vector<int>& arr, int left, int mid, int right) {
    vector<int> temp(right - left + 1);
    int i = left, j = mid + 1, k = 0;

    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }

    while (i <= mid) {
        temp[k++] = arr[i++];
    }
    while (j <= right) {
        temp[k++] = arr[j++];
    }

    for (int idx = 0; idx < temp.size(); idx++) {
        arr[left + idx] = temp[idx];
    }
}

void MergeSortTool(vector<int>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        MergeSortTool(arr, left, mid);
        MergeSortTool(arr, mid + 1, right);
        MergeTool(arr, left, mid, right);
    }
}

void MergeSort(vector<int>& arr) {
    if (arr.size() > 1) {
        MergeSortTool(arr, 0, arr.size() - 1);
    }
}

// Bucket Sort
void BucketSort(vector<int>& arr) {
    if (arr.empty()) return;

    int minVal = arr[0], maxVal = arr[0];
    for (int x : arr) {
        minVal = min(minVal, x);
        maxVal = max(maxVal, x);
    }

    int bucketCount = max(1, (int)arr.size() / 2);
    int bucketRange = (maxVal - minVal) / bucketCount + 1;

    vector<vector<int>> buckets(bucketCount);

    for (int x : arr) {
        int idx = (x - minVal) / bucketRange;
        idx = min(idx, bucketCount - 1);
        buckets[idx].push_back(x);
    }

    int idx = 0;
    for (auto& bucket : buckets) {
        if (!bucket.empty()) {
            sort(bucket.begin(), bucket.end());
            for (int x : bucket) {
                arr[idx++] = x;
            }
        }
    }
}

// Counting Sort
void CountingSort(vector<int>& arr) {
    if (arr.empty()) return;

    int minVal = arr[0], maxVal = arr[0];
    for (int x : arr) {
        minVal = min(minVal, x);
        maxVal = max(maxVal, x);
    }

    int range = maxVal - minVal + 1;
    vector<int> count(range, 0);

    for (int x : arr) {
        count[x - minVal]++;
    }

    int idx = 0;
    for (int i = 0; i < range; i++) {
        while (count[i] > 0) {
            arr[idx++] = i + minVal;
            count[i]--;
        }
    }
}

// Heap Sort
void Heapify(vector<int>& arr, int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && arr[left] > arr[largest]) {
        largest = left;
    }
    if (right < n && arr[right] > arr[largest]) {
        largest = right;
    }
    if (largest != i) {
        swap(arr[i], arr[largest]);
        Heapify(arr, n, largest);
    }
}

void HeapSort(vector<int>& arr) {
    int n = arr.size();

    // 建大顶堆
    for (int i = n / 2 - 1; i >= 0; i--) {
        Heapify(arr, n, i);
    }

    // 逐个提取最大值放到末尾
    for (int i = n - 1; i > 0; i--) {
        swap(arr[0], arr[i]);
        Heapify(arr, i, 0);
    }
}

void TestSortingAlgorithm(vector<int>& arr, void (*SortAlgorithm)(vector<int>&), const string& algorithmName) {
    cout << "========== " << algorithmName << " ==========" << endl;
    cout << "Source Array: ";
    ArrPrint(arr);

    // using chrono precise timing
    auto startTime = chrono::high_resolution_clock::now();
    SortAlgorithm(arr);
    auto endTime = chrono::high_resolution_clock::now();

    // computation time
    auto duration = chrono::duration_cast<chrono::microseconds>(endTime - startTime);
    double timeMs = duration.count() / 1000.0;

    cout << "Sorted Array: ";
    ArrPrint(arr);
    cout << "Time taken: " << timeMs << " ms (" << duration.count() << " us)" << endl;
    cout << "========================================\n" << endl;
}

int main() {
    // 随机生成100个1000以内的随机数
    const int ARRAY_SIZE = 1000;
    const int MAX_VALUE = 10000;
    
    // 使用 random 库生成高质量随机数
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(1, MAX_VALUE);
    
    vector<int> baseArr(ARRAY_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        baseArr[i] = distrib(gen);
    }
    
    // 为不同算法创建副本
    vector<int> arr1 = baseArr;
    vector<int> arr2 = baseArr;
    vector<int> arr3 = baseArr;
    vector<int> arr4 = baseArr;
    vector<int> arr5 = baseArr;
    vector<int> arr6 = baseArr;
    vector<int> arr7 = baseArr;
    vector<int> arr8 = baseArr;
    vector<int> arr9 = baseArr;

    // 使用统一的测试函数测试各排序算法
    TestSortingAlgorithm(arr1, BubbleSort, "BubbleSort");
    TestSortingAlgorithm(arr2, QuickSort, "QuickSort");
    TestSortingAlgorithm(arr3, InsertionSort, "InsertionSort");
    TestSortingAlgorithm(arr4, ShellSort, "ShellSort");
    TestSortingAlgorithm(arr5, SelectSort, "SelectSort");
    TestSortingAlgorithm(arr6, MergeSort, "MergeSort");
    TestSortingAlgorithm(arr7, BucketSort, "BucketSort");
    TestSortingAlgorithm(arr8, CountingSort, "CountingSort");
    TestSortingAlgorithm(arr9, HeapSort, "HeapSort");

    return 0;
}
