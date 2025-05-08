function deleteMatFilesCol(folderPath, startCol, endCol)
% 删除指定文件夹下所有MAT文件中从startCol到endCol的列
% 输入参数：
%   folderPath - 包含MAT文件的文件夹路径（字符串）
%   startCol   - 要删除的起始列号（正整数）
%   endCol     - 要删除的结束列号（正整数，≥startCol）

% 验证输入参数
if ~isfolder(folderPath)
    error('指定的文件夹不存在: %s', folderPath);
end

if ~isnumeric(startCol) || ~isnumeric(endCol) || startCol < 1 || endCol < startCol
    error('列号参数无效，应为正整数且endCol≥startCol');
end

% 获取所有.mat文件（包括子文件夹）
matFiles = dir(fullfile(folderPath, '**/*.mat'));

if isempty(matFiles)
    fprintf('文件夹中没有找到.mat文件: %s\n', folderPath);
    return;
end

fprintf('正在处理文件夹: %s\n', folderPath);
fprintf('找到 %d 个.mat文件\n', length(matFiles));
fprintf('将删除所有文件中第 %d 到 %d 列\n', startCol, endCol);

% 遍历所有MAT文件
for i = 1:length(matFiles)
    fileDir = matFiles(i).folder;
    currentFile = matFiles(i).name;
    fullPath = fullfile(fileDir, currentFile);
    
    try
        % 加载MAT文件数据
        matData = load(fullPath);
        vars = fieldnames(matData);
        modified = false;
        
        % 处理每个变量
        for j = 1:length(vars)
            varName = vars{j};
            varValue = matData.(varName);
            
            % 只处理二维矩阵
            if isnumeric(varValue) && ismatrix(varValue) && size(varValue,2) >= endCol
                % 删除指定列
                matData.(varName) = varValue(:, [1:startCol-1, endCol+1:end]);
                modified = true;
            end
        end
        
        if modified
            % 保存修改后的文件
            save(fullPath, '-struct', 'matData');
            fprintf('成功处理文件: %s\n', fullPath);
        else
            fprintf('文件 %s 中没有符合条件的矩阵，跳过...\n', fullPath);
        end
        
    catch ME
        fprintf('处理文件 %s 时出错: %s\n', fullPath, ME.message);
    end
end

fprintf('处理完成！\n');
end