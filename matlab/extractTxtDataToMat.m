function extractTxtDataToMat(folderPath, outputFolder)
% 将每个TXT文件转换为对应的MAT文件
% 输入：
%   folderPath - 包含TXT文件的文件夹路径
%   outputFolder - 输出MAT文件的文件夹路径

% 获取所有TXT文件
fileList = dir(fullfile(folderPath, '*.txt'));
if isempty(fileList)
    error('未找到TXT文件: %s', folderPath);
end

% 创建输出文件夹（如果不存在）
if ~exist(outputFolder, 'dir')
    mkdir(outputFolder);
end

% 处理每个文件
for i = 1:length(fileList)
    currentFile = fullfile(fileList(i).folder, fileList(i).name);
    fprintf('正在处理: %s\n', currentFile);
    
    try
        % 读取文件内容
        fileContent = fileread(currentFile);
        
        % 按行分割
        lines = strsplit(fileContent, '\n');
        
        % 初始化数据存储
        extractedData = [];
        
        % 处理每一行
        for j = 1:length(lines)
            line = strtrim(lines{j});
            if isempty(line)
                continue;
            end
            
            % 使用正则表达式提取两个浮点数
            tokens = regexp(line, '(-?\d+\.\d+)', 'tokens');
            
            % 验证提取结果
            if length(tokens) >= 3  % 至少需要3个数字（跳过时间戳）
                % 提取第2和第3个数值（跳过时间戳后的第一个数字）
                num1 = str2double(tokens{1}{1});
                num2 = str2double(tokens{2}{1});
                
                if ~isnan(num1) && ~isnan(num2)
                    extractedData = [extractedData; num1, num2];
                end
            end
        end
        
        % 保存到MAT文件
        if ~isempty(extractedData)
            % 生成输出文件名（与TXT文件同名，扩展名改为.mat）
            [~, name, ~] = fileparts(currentFile);
            outputFile = fullfile(outputFolder, [name '.mat']);
            
            save(outputFile, 'extractedData');
            fprintf('  成功保存 %d 组数据到: %s\n', size(extractedData, 1), outputFile);
        else
            warning('  文件 %s 未提取到有效数据', currentFile);
        end
        
    catch ME
        warning('处理文件 %s 失败: %s', currentFile, ME.message);
    end
end

fprintf('转换完成！共处理 %d 个文件\n', length(fileList));
end