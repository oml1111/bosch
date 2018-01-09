setwd("D:\\Development\\Data Mining Poster Session")

data = read.csv("train_combined.csv")
data[1:100,1]

library(caret)

ctrl <- trainControl(method = "cv", p = 0.8)
svmLinear_fit <- train(Response ~ ., data = data, method = "svmLinear", trControl = ctrl, preProc = c("center", "scale"))
