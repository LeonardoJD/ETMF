#coding = utf-8
from numpy import *
import matplotlib.pyplot as plt
import sys
import math
import random

USERS = 6040
ITEMS = 3952
ENTROPY_THRESHOLD = 1.0

def changeFileFormat(filename, newFileName):        #转换原始文件的格式
    fold = open(filename)
    fnew = open(newFileName,'w')
    arrayOfLines = fold.readlines()
    for line in arrayOfLines:
        line = line.strip()
        listFromLine = line.split("::")
        newLine = '\t'.join(listFromLine)
        fnew.write(newLine)
        fnew.write('\n')
    fold.close()
    fnew.close()
    

def file2Matrix(filename):           #从文件读取数据，存储为矩阵
    fread = open(filename)
    arrayOfLines = fread.readlines()
    numberOfLines = len(arrayOfLines)
    returnMat = zeros((numberOfLines,4))
    index = 0
    for line in arrayOfLines:
        line = line.strip()
        listFromLine = line.split('\t')
        returnMat[index,:] = listFromLine[0:4]
        index += 1
    return returnMat

def getRatingMatrix(dataMat):          #将数据转化为评分矩阵
    ratingMat = zeros((USERS, ITEMS))
    for rating in dataMat:
        ratingMat[rating[0]-1][rating[1]-1] = rating[2]
    return ratingMat

def calEntropy(ratingMat):          #计算用户信息熵
    spamIndex = []
    m,n = shape(ratingMat)
    dictUsersEntropy = {}.fromkeys(range(1,m+1))
    index = 1
    for rating in ratingMat:
        rating = list(rating)
        nums = zeros(5)
        sum = 0
        entropy = 0.0
        for i in range(1,6):
            nums[i-1] = rating.count(i)
        for i in nums:
            sum += i
        for i in nums:
            if i != 0:
               entropy -= (i/sum) * math.log(i/sum, 2)
        if  entropy < ENTROPY_THRESHOLD:
            spamIndex.append(index)
        dictUsersEntropy[index] = round(entropy,4)
        index += 1
    return spamIndex,dictUsersEntropy

def eliminateSpam(filename,newFileName):      #除去噪音数据
     dataMat = file2Matrix(filename)
     ratingMat = getRatingMatrix(dataMat)
     spamUserIndex,dictEntropy = calEntropy(ratingMat)
     spamItemIndex = []
     fnew = open(newFileName, 'w')
     for line in dataMat:
         strLine = [str(int(i)) for i in line ]
         if (line[0] not in spamUserIndex):
             fnew.write('\t'.join(strLine))
             fnew.write('\n')
     fnew.close()
     nLeftUsers,nLeftItems = renumberIndex(newFileName)
     split(newFileName)
     return nLeftUsers,nLeftItems
    
def renumberIndex(filename):               #对用户和项目重新进行编号，防止编号不连续的情况
    dataMat = file2Matrix(filename)
    oldUsersIndex = []
    oldItemsIndex = []
    for rating in dataMat:
        oldUsersIndex.append(int(rating[0]))
        oldItemsIndex.append(int(rating[1]))
    uniqueUsersIndex = list(set(oldUsersIndex))
    uniqueItemsIndex = list(set(oldItemsIndex))
    nUsers = len(uniqueUsersIndex)
    nItems = len(uniqueItemsIndex)
    dictUsersIndex = {}.fromkeys(uniqueUsersIndex)
    dictItemsIndex = {}.fromkeys(uniqueItemsIndex)
    newUserIndex = 1
    newItemIndex = 1
    for i in uniqueUsersIndex:
        dictUsersIndex[i] = newUserIndex
        newUserIndex += 1
    for i in uniqueItemsIndex:
        dictItemsIndex[i] = newItemIndex
        newItemIndex += 1
    for rating in dataMat:
        rating[0] = dictUsersIndex[rating[0]]
        rating[1] = dictItemsIndex[rating[1]]
    savetxt(filename, dataMat, fmt = "%d", delimiter = '\t')
    return nUsers,nItems


def split(filename, trainFileName="train.dat", testFileName="test.dat", ratio = 0.2):    #将原始数据分割为训练集和测试集
    arrayOfLines =  open(filename).readlines()
    length = len(arrayOfLines)
    indexes = range(0, length)
    count = int(ratio * length)
    testIndex = random.sample(range(0,length), count)
    trainIndex = list(set(indexes)^set(testIndex))
    ftest = open(testFileName, 'w')
    for i in testIndex:
        ftest.write(arrayOfLines[i])
    ftest.close()
    ftrain = open(trainFileName, 'w')
    for i in trainIndex:
        ftrain.write(arrayOfLines[i])
    ftrain.close()

def plotEntropy(filename):      #画出用户信息熵分布图
    dataMat = file2Matrix(filename)
    ratingMat = getRatingMatrix(dataMat)
    spamIndex,dictUsersEntropy = calEntropy(ratingMat)
    X = dictUsersEntropy.keys()
    Y = dictUsersEntropy.values()
    #plt.xlim(0,7000)
    #plt.ylim(0,2,5)
    plt.grid(True)
    plt.scatter(X,Y)
    plt.xlabel("User ID")
    plt.ylabel("The Entropy of Users")
    plt.savefig("entropy2.eps")

def plotEntropyThreshold():     #画出不同信息熵阈值下的RMSE值
    X = [0.0,0.5,0.6,0.7,0.8,0.9,1.0,1.1,1.2,1.3,1.4,1.5]
    Y = [0.8713,0.8703,0.8700,0.8698,0.8695,0.8690,0.8652,0.8620,0.8667,0.8703,0.8796,0.8892]
    plt.plot(X,Y)
    plt.xlabel("Entropy Threshold")
    plt.ylabel("RMSE")
    plt.grid(True)
    plt.show()
   # plt.savefig("threshold3.eps")
    



"""if __name__ == '__main__':
  #  plotEntropy("newRatings.dat")
  #  nLeftUsers,nLeftItems = eliminateSpam("newRatings.dat", "filteredRatings.dat")
  #  print nLeftUsers,nLeftItems
   for i in range(1,11):
        trainFile = "train%d.dat" % i
        testFile = "test%d.dat" % i
        split("newRatings.dat", trainFile, testFile)"""

    
  


            
            
        
        
            

  
