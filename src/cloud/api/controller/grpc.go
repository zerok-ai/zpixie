package controller

import (
	"context"
	"fmt"
	"io/ioutil"
	"path/filepath"

	"github.com/gofrs/uuid"
	"github.com/gogo/protobuf/types"
	"github.com/spf13/pflag"
	"github.com/spf13/viper"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/metadata"
	"google.golang.org/grpc/status"

	public_cloudapipb "pixielabs.ai/pixielabs/src/api/public/cloudapipb"
	"pixielabs.ai/pixielabs/src/api/public/uuidpb"
	"pixielabs.ai/pixielabs/src/cloud/artifact_tracker/artifacttrackerpb"
	authpb "pixielabs.ai/pixielabs/src/cloud/auth/proto"
	"pixielabs.ai/pixielabs/src/cloud/autocomplete"
	"pixielabs.ai/pixielabs/src/cloud/cloudapipb"
	profilepb "pixielabs.ai/pixielabs/src/cloud/profile/profilepb"
	"pixielabs.ai/pixielabs/src/cloud/scriptmgr/scriptmgrpb"
	"pixielabs.ai/pixielabs/src/cloud/vzmgr/vzmgrpb"
	"pixielabs.ai/pixielabs/src/shared/artifacts/versionspb"
	"pixielabs.ai/pixielabs/src/shared/cvmsgspb"
	"pixielabs.ai/pixielabs/src/shared/services/authcontext"
	"pixielabs.ai/pixielabs/src/shared/services/utils"
	pbutils "pixielabs.ai/pixielabs/src/utils"
)

func init() {
	pflag.String("vizier_image_secret_path", "/vizier-image-secret", "[WORKAROUND] The path the the image secrets")
	pflag.String("vizier_image_secret_file", "vizier_image_secret.json", "[WORKAROUND] The image secret file")
}

// VizierImageAuthServer is the GRPC server responsible for providing access to Vizier images.
type VizierImageAuthServer struct{}

// GetImageCredentials fetches image credentials for vizier.
func (v VizierImageAuthServer) GetImageCredentials(context.Context, *cloudapipb.GetImageCredentialsRequest) (*cloudapipb.GetImageCredentialsResponse, error) {
	// These creds are just for internal use. All official releases are written to a public registry.
	p := viper.GetString("vizier_image_secret_path")
	f := viper.GetString("vizier_image_secret_file")

	absP, err := filepath.Abs(p)
	if err != nil {
		return nil, status.Error(codes.Internal, "failed to parse creds paths")
	}
	credsFile := filepath.Join(absP, f)
	b, err := ioutil.ReadFile(credsFile)
	if err != nil {
		return nil, status.Error(codes.Internal, "failed to read creds file")
	}

	return &cloudapipb.GetImageCredentialsResponse{Creds: string(b)}, nil
}

// ArtifactTrackerServer is the GRPC server responsible for providing access to artifacts.
type ArtifactTrackerServer struct {
	ArtifactTrackerClient artifacttrackerpb.ArtifactTrackerClient
}

func getArtifactTypeFromCloudProto(a cloudapipb.ArtifactType) versionspb.ArtifactType {
	switch a {
	case cloudapipb.AT_LINUX_AMD64:
		return versionspb.AT_LINUX_AMD64
	case cloudapipb.AT_DARWIN_AMD64:
		return versionspb.AT_DARWIN_AMD64
	case cloudapipb.AT_CONTAINER_SET_YAMLS:
		return versionspb.AT_CONTAINER_SET_YAMLS
	case cloudapipb.AT_CONTAINER_SET_LINUX_AMD64:
		return versionspb.AT_CONTAINER_SET_LINUX_AMD64
	case cloudapipb.AT_CONTAINER_SET_TEMPLATE_YAMLS:
		return versionspb.AT_CONTAINER_SET_TEMPLATE_YAMLS
	default:
		return versionspb.AT_UNKNOWN
	}
}

func getArtifactTypeFromVersionsProto(a versionspb.ArtifactType) cloudapipb.ArtifactType {
	switch a {
	case versionspb.AT_LINUX_AMD64:
		return cloudapipb.AT_LINUX_AMD64
	case versionspb.AT_DARWIN_AMD64:
		return cloudapipb.AT_DARWIN_AMD64
	case versionspb.AT_CONTAINER_SET_YAMLS:
		return cloudapipb.AT_CONTAINER_SET_YAMLS
	case versionspb.AT_CONTAINER_SET_LINUX_AMD64:
		return cloudapipb.AT_CONTAINER_SET_LINUX_AMD64
	case versionspb.AT_CONTAINER_SET_TEMPLATE_YAMLS:
		return cloudapipb.AT_CONTAINER_SET_TEMPLATE_YAMLS
	default:
		return cloudapipb.AT_UNKNOWN
	}
}

func getServiceCredentials(signingKey string) (string, error) {
	claims := utils.GenerateJWTForService("API Service", viper.GetString("domain_name"))
	return utils.SignJWTClaims(claims, signingKey)
}

// GetArtifactList gets the set of artifact versions for the given artifact.
func (a ArtifactTrackerServer) GetArtifactList(ctx context.Context, req *cloudapipb.GetArtifactListRequest) (*cloudapipb.ArtifactSet, error) {
	atReq := &artifacttrackerpb.GetArtifactListRequest{
		ArtifactType: getArtifactTypeFromCloudProto(req.ArtifactType),
		ArtifactName: req.ArtifactName,
		Limit:        req.Limit,
	}

	serviceAuthToken, err := getServiceCredentials(viper.GetString("jwt_signing_key"))
	if err != nil {
		return nil, err
	}
	ctx = metadata.AppendToOutgoingContext(ctx, "authorization",
		fmt.Sprintf("bearer %s", serviceAuthToken))

	resp, err := a.ArtifactTrackerClient.GetArtifactList(ctx, atReq)
	if err != nil {
		return nil, err
	}

	cloudpbArtifacts := make([]*cloudapipb.Artifact, len(resp.Artifact))
	for i, artifact := range resp.Artifact {
		availableArtifacts := make([]cloudapipb.ArtifactType, len(artifact.AvailableArtifacts))
		for j, a := range artifact.AvailableArtifacts {
			availableArtifacts[j] = getArtifactTypeFromVersionsProto(a)
		}
		cloudpbArtifacts[i] = &cloudapipb.Artifact{
			Timestamp:          artifact.Timestamp,
			CommitHash:         artifact.CommitHash,
			VersionStr:         artifact.VersionStr,
			Changelog:          artifact.Changelog,
			AvailableArtifacts: availableArtifacts,
		}
	}

	return &cloudapipb.ArtifactSet{
		Name:     resp.Name,
		Artifact: cloudpbArtifacts,
	}, nil
}

// GetDownloadLink gets the download link for the given artifact.
func (a ArtifactTrackerServer) GetDownloadLink(ctx context.Context, req *cloudapipb.GetDownloadLinkRequest) (*cloudapipb.GetDownloadLinkResponse, error) {
	atReq := &artifacttrackerpb.GetDownloadLinkRequest{
		ArtifactName: req.ArtifactName,
		VersionStr:   req.VersionStr,
		ArtifactType: getArtifactTypeFromCloudProto(req.ArtifactType),
	}

	serviceAuthToken, err := getServiceCredentials(viper.GetString("jwt_signing_key"))
	if err != nil {
		return nil, err
	}
	ctx = metadata.AppendToOutgoingContext(ctx, "authorization",
		fmt.Sprintf("bearer %s", serviceAuthToken))

	resp, err := a.ArtifactTrackerClient.GetDownloadLink(ctx, atReq)
	if err != nil {
		return nil, err
	}

	return &cloudapipb.GetDownloadLinkResponse{
		Url:        resp.Url,
		SHA256:     resp.SHA256,
		ValidUntil: resp.ValidUntil,
	}, nil
}

// VizierClusterInfo is the server that implements the VizierClusterInfo gRPC service.
type VizierClusterInfo struct {
	VzMgr                 vzmgrpb.VZMgrServiceClient
	ArtifactTrackerClient artifacttrackerpb.ArtifactTrackerClient
}

func contextWithAuthToken(ctx context.Context) (context.Context, error) {
	sCtx, err := authcontext.FromContext(ctx)
	if err != nil {
		return nil, err
	}
	return metadata.AppendToOutgoingContext(ctx, "authorization",
		fmt.Sprintf("bearer %s", sCtx.AuthToken)), nil
}

// CreateCluster creates a cluster for the current org.
func (v *VizierClusterInfo) CreateCluster(ctx context.Context, request *cloudapipb.CreateClusterRequest) (*cloudapipb.CreateClusterResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "Deprecated. Please use `px deploy`")
}

// GetClusterInfo returns information about Vizier clusters.
func (v *VizierClusterInfo) GetClusterInfo(ctx context.Context, request *cloudapipb.GetClusterInfoRequest) (*cloudapipb.GetClusterInfoResponse, error) {
	sCtx, err := authcontext.FromContext(ctx)
	if err != nil {
		return nil, err
	}
	orgIDstr := sCtx.Claims.GetUserClaims().OrgID
	orgID, err := uuid.FromString(orgIDstr)
	if err != nil {
		return nil, err
	}

	ctx, err = contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}

	vzIDs := make([]*uuidpb.UUID, 0)
	if request.ID != nil {
		vzIDs = append(vzIDs, request.ID)
	} else {
		viziers, err := v.VzMgr.GetViziersByOrg(ctx, pbutils.ProtoFromUUID(orgID))
		if err != nil {
			return nil, err
		}
		vzIDs = viziers.VizierIDs
	}

	return v.getClusterInfoForViziers(ctx, vzIDs)
}

func (v *VizierClusterInfo) getClusterInfoForViziers(ctx context.Context, ids []*uuidpb.UUID) (*cloudapipb.GetClusterInfoResponse, error) {
	resp := &cloudapipb.GetClusterInfoResponse{}

	cNames := make(map[string]int)
	vzInfoResp, err := v.VzMgr.GetVizierInfos(ctx, &vzmgrpb.GetVizierInfosRequest{
		VizierIDs: ids,
	})

	if err != nil {
		return nil, err
	}

	for _, vzInfo := range vzInfoResp.VizierInfos {
		if vzInfo == nil || vzInfo.VizierID == nil {
			continue
		}
		podStatuses := make(map[string]*cloudapipb.PodStatus)
		for podName, status := range vzInfo.ControlPlanePodStatuses {
			var containers []*cloudapipb.ContainerStatus
			for _, container := range status.Containers {
				containers = append(containers, &cloudapipb.ContainerStatus{
					Name:      container.Name,
					State:     container.State,
					Message:   container.Message,
					Reason:    container.Reason,
					CreatedAt: container.CreatedAt,
				})
			}
			var events []*cloudapipb.K8SEvent
			for _, ev := range status.Events {
				events = append(events, &cloudapipb.K8SEvent{
					Message:   ev.Message,
					LastTime:  ev.LastTime,
					FirstTime: ev.FirstTime,
				})
			}

			podStatuses[podName] = &cloudapipb.PodStatus{
				Name:          status.Name,
				Status:        status.Status,
				StatusMessage: status.StatusMessage,
				Reason:        status.Reason,
				Containers:    containers,
				CreatedAt:     status.CreatedAt,
				Events:        events,
			}
		}

		s := vzStatusToClusterStatus(vzInfo.Status)
		prettyName := PrettifyClusterName(vzInfo.ClusterName, false)

		if val, ok := cNames[prettyName]; ok {
			cNames[prettyName] = val + 1
		} else {
			cNames[prettyName] = 1
		}

		resp.Clusters = append(resp.Clusters, &cloudapipb.ClusterInfo{
			ID:              vzInfo.VizierID,
			Status:          s,
			LastHeartbeatNs: vzInfo.LastHeartbeatNs,
			Config: &cloudapipb.VizierConfig{
				PassthroughEnabled: vzInfo.Config.PassthroughEnabled,
				AutoUpdateEnabled:  vzInfo.Config.AutoUpdateEnabled,
			},
			ClusterUID:              vzInfo.ClusterUID,
			ClusterName:             vzInfo.ClusterName,
			PrettyClusterName:       prettyName,
			ClusterVersion:          vzInfo.ClusterVersion,
			VizierVersion:           vzInfo.VizierVersion,
			ControlPlanePodStatuses: podStatuses,
			NumNodes:                vzInfo.NumNodes,
			NumInstrumentedNodes:    vzInfo.NumInstrumentedNodes,
		})
	}

	// For duplicate prettyNames, update the prettyNames to have more context.
	for i, c := range resp.Clusters {
		if cNames[c.PrettyClusterName] > 1 {
			resp.Clusters[i].PrettyClusterName = PrettifyClusterName(c.ClusterName, true)
		}
	}

	return resp, nil
}

// GetClusterConnectionInfo returns information about connections to Vizier cluster.
func (v *VizierClusterInfo) GetClusterConnectionInfo(ctx context.Context, request *cloudapipb.GetClusterConnectionInfoRequest) (*cloudapipb.GetClusterConnectionInfoResponse, error) {
	id := request.ID
	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}

	ci, err := v.VzMgr.GetVizierConnectionInfo(ctx, id)
	if err != nil {
		return nil, err
	}

	return &cloudapipb.GetClusterConnectionInfoResponse{
		IPAddress: ci.IPAddress,
		Token:     ci.Token,
	}, nil
}

// UpdateClusterVizierConfig supports updates of VizierConfig for a cluster
func (v *VizierClusterInfo) UpdateClusterVizierConfig(ctx context.Context, req *cloudapipb.UpdateClusterVizierConfigRequest) (*cloudapipb.UpdateClusterVizierConfigResponse, error) {
	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}

	_, err = v.VzMgr.UpdateVizierConfig(ctx, &cvmsgspb.UpdateVizierConfigRequest{
		VizierID: req.ID,
		ConfigUpdate: &cvmsgspb.VizierConfigUpdate{
			PassthroughEnabled: req.ConfigUpdate.PassthroughEnabled,
			AutoUpdateEnabled:  req.ConfigUpdate.AutoUpdateEnabled,
		},
	})
	if err != nil {
		return nil, err
	}

	return &cloudapipb.UpdateClusterVizierConfigResponse{}, nil
}

// UpdateClusterConfig supports updates of config for a cluster
func (v *VizierClusterInfo) UpdateClusterConfig(ctx context.Context, req *public_cloudapipb.UpdateClusterConfigRequest) (*public_cloudapipb.UpdateClusterConfigResponse, error) {
	_, err := v.UpdateClusterVizierConfig(ctx, &cloudapipb.UpdateClusterVizierConfigRequest{
		ID: req.ID,
		ConfigUpdate: &cloudapipb.VizierConfigUpdate{
			PassthroughEnabled: req.ConfigUpdate.PassthroughEnabled,
			AutoUpdateEnabled:  req.ConfigUpdate.AutoUpdateEnabled,
		},
	})

	if err != nil {
		return nil, err
	}

	return &public_cloudapipb.UpdateClusterConfigResponse{}, nil
}

func vizierStatusToPublicVizierStatus(s cloudapipb.ClusterStatus) public_cloudapipb.ClusterStatus {
	switch s {
	case cloudapipb.CS_HEALTHY:
		return public_cloudapipb.CS_HEALTHY
	case cloudapipb.CS_UNHEALTHY:
		return public_cloudapipb.CS_UNHEALTHY
	case cloudapipb.CS_DISCONNECTED:
		return public_cloudapipb.CS_DISCONNECTED
	case cloudapipb.CS_UPDATING:
		return public_cloudapipb.CS_UPDATING
	case cloudapipb.CS_CONNECTED:
		return public_cloudapipb.CS_CONNECTED
	case cloudapipb.CS_UPDATE_FAILED:
		return public_cloudapipb.CS_UPDATE_FAILED
	default:
		return public_cloudapipb.CS_UNKNOWN
	}
}

// GetCluster gets status info about the specified vizier.
func (v *VizierClusterInfo) GetCluster(ctx context.Context, req *public_cloudapipb.GetClusterRequest) (*public_cloudapipb.GetClusterResponse, error) {
	resp, err := v.GetClusterInfo(ctx, &cloudapipb.GetClusterInfoRequest{
		ID: req.ID,
	})

	if err != nil {
		return nil, err
	}

	clusters := make([]*public_cloudapipb.ClusterInfo, len(resp.Clusters))
	for i, c := range resp.Clusters {
		clusters[i] = &public_cloudapipb.ClusterInfo{
			ID:              c.ID,
			Status:          vizierStatusToPublicVizierStatus(c.Status),
			LastHeartbeatNs: c.LastHeartbeatNs,
			Config: &public_cloudapipb.ClusterConfig{
				PassthroughEnabled: c.Config.PassthroughEnabled,
				AutoUpdateEnabled:  c.Config.AutoUpdateEnabled,
			},
			ClusterName:          c.ClusterName,
			ClusterVersion:       c.ClusterVersion,
			VizierVersion:        c.VizierVersion,
			NumNodes:             c.NumNodes,
			NumInstrumentedNodes: c.NumInstrumentedNodes,
		}
	}

	return &public_cloudapipb.GetClusterResponse{
		Clusters: clusters,
	}, nil
}

// GetClusterConnection is the public-facing call to get a cluster's connection info.
func (v *VizierClusterInfo) GetClusterConnection(ctx context.Context, req *public_cloudapipb.GetClusterConnectionRequest) (*public_cloudapipb.GetClusterConnectionResponse, error) {
	resp, err := v.GetClusterConnectionInfo(ctx, &cloudapipb.GetClusterConnectionInfoRequest{
		ID: req.ID,
	})

	if err != nil {
		return nil, err
	}

	return &public_cloudapipb.GetClusterConnectionResponse{
		IPAddress: resp.IPAddress,
		Token:     resp.Token,
	}, nil
}

// UpdateOrInstallCluster updates or installs the given vizier cluster to the specified version.
func (v *VizierClusterInfo) UpdateOrInstallCluster(ctx context.Context, req *cloudapipb.UpdateOrInstallClusterRequest) (*cloudapipb.UpdateOrInstallClusterResponse, error) {
	if req.Version == "" {
		return nil, status.Errorf(codes.InvalidArgument, "version cannot be empty")
	}

	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}

	// Validate version.
	atReq := &artifacttrackerpb.GetDownloadLinkRequest{
		ArtifactName: "vizier",
		VersionStr:   req.Version,
		ArtifactType: versionspb.AT_CONTAINER_SET_YAMLS,
	}

	_, err = v.ArtifactTrackerClient.GetDownloadLink(ctx, atReq)
	if err != nil {
		return nil, status.Errorf(codes.InvalidArgument, "invalid version")
	}

	resp, err := v.VzMgr.UpdateOrInstallVizier(ctx, &cvmsgspb.UpdateOrInstallVizierRequest{
		VizierID:     req.ClusterID,
		Version:      req.Version,
		RedeployEtcd: req.RedeployEtcd,
	})
	if err != nil {
		return nil, err
	}

	return &cloudapipb.UpdateOrInstallClusterResponse{
		UpdateStarted: resp.UpdateStarted,
	}, nil
}

func vzStatusToClusterStatus(s cvmsgspb.VizierStatus) cloudapipb.ClusterStatus {
	switch s {
	case cvmsgspb.VZ_ST_HEALTHY:
		return cloudapipb.CS_HEALTHY
	case cvmsgspb.VZ_ST_UNHEALTHY:
		return cloudapipb.CS_UNHEALTHY
	case cvmsgspb.VZ_ST_DISCONNECTED:
		return cloudapipb.CS_DISCONNECTED
	case cvmsgspb.VZ_ST_UPDATING:
		return cloudapipb.CS_UPDATING
	case cvmsgspb.VZ_ST_CONNECTED:
		return cloudapipb.CS_CONNECTED
	case cvmsgspb.VZ_ST_UPDATE_FAILED:
		return cloudapipb.CS_UPDATE_FAILED
	default:
		return cloudapipb.CS_UNKNOWN
	}
}

// VizierDeploymentKeyServer is the server that implements the VizierDeploymentKeyManager gRPC service.
type VizierDeploymentKeyServer struct {
	VzDeploymentKey vzmgrpb.VZDeploymentKeyServiceClient
}

func deployKeyToCloudAPI(key *vzmgrpb.DeploymentKey) *cloudapipb.DeploymentKey {
	return &cloudapipb.DeploymentKey{
		ID:        key.ID,
		Key:       key.Key,
		CreatedAt: key.CreatedAt,
		Desc:      key.Desc,
	}
}

// Create creates a new deploy key in vzmgr.
func (v *VizierDeploymentKeyServer) Create(ctx context.Context, req *cloudapipb.CreateDeploymentKeyRequest) (*cloudapipb.DeploymentKey, error) {
	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}

	resp, err := v.VzDeploymentKey.Create(ctx, &vzmgrpb.CreateDeploymentKeyRequest{Desc: req.Desc})
	if err != nil {
		return nil, err
	}
	return deployKeyToCloudAPI(resp), nil
}

// List lists all of the deploy keys in vzmgr.
func (v *VizierDeploymentKeyServer) List(ctx context.Context, req *cloudapipb.ListDeploymentKeyRequest) (*cloudapipb.ListDeploymentKeyResponse, error) {
	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}

	resp, err := v.VzDeploymentKey.List(ctx, &vzmgrpb.ListDeploymentKeyRequest{})
	if err != nil {
		return nil, err
	}
	var keys []*cloudapipb.DeploymentKey
	for _, key := range resp.Keys {
		keys = append(keys, deployKeyToCloudAPI(key))
	}
	return &cloudapipb.ListDeploymentKeyResponse{
		Keys: keys,
	}, nil
}

// Get fetches a specific deploy key in vzmgr.
func (v *VizierDeploymentKeyServer) Get(ctx context.Context, req *cloudapipb.GetDeploymentKeyRequest) (*cloudapipb.GetDeploymentKeyResponse, error) {
	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}

	resp, err := v.VzDeploymentKey.Get(ctx, &vzmgrpb.GetDeploymentKeyRequest{
		ID: req.ID,
	})
	if err != nil {
		return nil, err
	}
	return &cloudapipb.GetDeploymentKeyResponse{
		Key: deployKeyToCloudAPI(resp.Key),
	}, nil
}

// Delete deletes a specific deploy key in vzmgr.
func (v *VizierDeploymentKeyServer) Delete(ctx context.Context, uuid *uuidpb.UUID) (*types.Empty, error) {
	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}
	return v.VzDeploymentKey.Delete(ctx, uuid)
}

// APIKeyServer is the server that implements the APIKeyManager gRPC service.
type APIKeyServer struct {
	APIKeyClient authpb.APIKeyServiceClient
}

func apiKeyToCloudAPI(key *authpb.APIKey) *cloudapipb.APIKey {
	return &cloudapipb.APIKey{
		ID:        key.ID,
		Key:       key.Key,
		CreatedAt: key.CreatedAt,
		Desc:      key.Desc,
	}
}

// Create creates a new API key.
func (v *APIKeyServer) Create(ctx context.Context, req *cloudapipb.CreateAPIKeyRequest) (*cloudapipb.APIKey, error) {
	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}

	resp, err := v.APIKeyClient.Create(ctx, &authpb.CreateAPIKeyRequest{Desc: req.Desc})
	if err != nil {
		return nil, err
	}
	return apiKeyToCloudAPI(resp), nil
}

// List lists all of the API keys in vzmgr.
func (v *APIKeyServer) List(ctx context.Context, req *cloudapipb.ListAPIKeyRequest) (*cloudapipb.ListAPIKeyResponse, error) {
	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}

	resp, err := v.APIKeyClient.List(ctx, &authpb.ListAPIKeyRequest{})
	if err != nil {
		return nil, err
	}
	var keys []*cloudapipb.APIKey
	for _, key := range resp.Keys {
		keys = append(keys, apiKeyToCloudAPI(key))
	}
	return &cloudapipb.ListAPIKeyResponse{
		Keys: keys,
	}, nil
}

// Get fetches a specific API key.
func (v *APIKeyServer) Get(ctx context.Context, req *cloudapipb.GetAPIKeyRequest) (*cloudapipb.GetAPIKeyResponse, error) {
	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}

	resp, err := v.APIKeyClient.Get(ctx, &authpb.GetAPIKeyRequest{
		ID: req.ID,
	})
	if err != nil {
		return nil, err
	}
	return &cloudapipb.GetAPIKeyResponse{
		Key: apiKeyToCloudAPI(resp.Key),
	}, nil
}

// Delete deletes a specific API key.
func (v *APIKeyServer) Delete(ctx context.Context, uuid *uuidpb.UUID) (*types.Empty, error) {
	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}
	return v.APIKeyClient.Delete(ctx, uuid)
}

// AutocompleteServer is the server that implements the Autocomplete gRPC service.
type AutocompleteServer struct {
	Suggester autocomplete.Suggester
}

// Autocomplete returns a formatted string and autocomplete suggestions.
func (a *AutocompleteServer) Autocomplete(ctx context.Context, req *cloudapipb.AutocompleteRequest) (*cloudapipb.AutocompleteResponse, error) {
	sCtx, err := authcontext.FromContext(ctx)
	if err != nil {
		return nil, err
	}
	orgIDstr := sCtx.Claims.GetUserClaims().OrgID
	orgID, err := uuid.FromString(orgIDstr)
	if err != nil {
		return nil, err
	}

	fmtString, executable, suggestions, err := autocomplete.Autocomplete(req.Input, int(req.CursorPos), req.Action, a.Suggester, orgID, req.ClusterUID)
	if err != nil {
		return nil, err
	}

	return &cloudapipb.AutocompleteResponse{
		FormattedInput: fmtString,
		IsExecutable:   executable,
		TabSuggestions: suggestions,
	}, nil
}

// AutocompleteField returns suggestions for a single field.
func (a *AutocompleteServer) AutocompleteField(ctx context.Context, req *cloudapipb.AutocompleteFieldRequest) (*cloudapipb.AutocompleteFieldResponse, error) {
	sCtx, err := authcontext.FromContext(ctx)
	if err != nil {
		return nil, err
	}
	orgIDstr := sCtx.Claims.GetUserClaims().OrgID
	orgID, err := uuid.FromString(orgIDstr)
	if err != nil {
		return nil, err
	}

	allowedArgs := []cloudapipb.AutocompleteEntityKind{}
	if req.RequiredArgTypes != nil {
		allowedArgs = req.RequiredArgTypes
	}

	suggestionReq := []*autocomplete.SuggestionRequest{
		{
			OrgID:        orgID,
			Input:        req.Input,
			AllowedKinds: []cloudapipb.AutocompleteEntityKind{req.FieldType},
			AllowedArgs:  allowedArgs,
			ClusterUID:   req.ClusterUID,
		},
	}
	suggestions, err := a.Suggester.GetSuggestions(suggestionReq)
	if err != nil {
		return nil, err
	}
	if len(suggestions) != 1 {
		return nil, status.Error(codes.Internal, "failed to get autocomplete suggestions")
	}

	acSugg := make([]*cloudapipb.AutocompleteSuggestion, len(suggestions[0].Suggestions))
	for j, s := range suggestions[0].Suggestions {
		acSugg[j] = &cloudapipb.AutocompleteSuggestion{
			Kind:           s.Kind,
			Name:           s.Name,
			Description:    s.Desc,
			MatchedIndexes: s.MatchedIndexes,
			State:          s.State,
		}
	}

	return &cloudapipb.AutocompleteFieldResponse{
		Suggestions: acSugg,
	}, nil
}

// ScriptMgrServer is the server that implements the ScriptMgr gRPC service.
type ScriptMgrServer struct {
	ScriptMgr scriptmgrpb.ScriptMgrServiceClient
}

// GetLiveViews returns a list of all available live views.
func (s *ScriptMgrServer) GetLiveViews(ctx context.Context, req *cloudapipb.GetLiveViewsReq) (*cloudapipb.GetLiveViewsResp, error) {
	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}
	smReq := &scriptmgrpb.GetLiveViewsReq{}
	smResp, err := s.ScriptMgr.GetLiveViews(ctx, smReq)
	if err != nil {
		return nil, err
	}
	resp := &cloudapipb.GetLiveViewsResp{
		LiveViews: make([]*cloudapipb.LiveViewMetadata, len(smResp.LiveViews)),
	}
	for i, liveView := range smResp.LiveViews {
		resp.LiveViews[i] = &cloudapipb.LiveViewMetadata{
			ID:   pbutils.UUIDFromProtoOrNil(liveView.ID).String(),
			Name: liveView.Name,
			Desc: liveView.Desc,
		}
	}
	return resp, nil
}

// GetLiveViewContents returns the pxl script, vis info, and metdata for a live view.
func (s *ScriptMgrServer) GetLiveViewContents(ctx context.Context, req *cloudapipb.GetLiveViewContentsReq) (*cloudapipb.GetLiveViewContentsResp, error) {
	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}

	smReq := &scriptmgrpb.GetLiveViewContentsReq{
		LiveViewID: pbutils.ProtoFromUUIDStrOrNil(req.LiveViewID),
	}
	smResp, err := s.ScriptMgr.GetLiveViewContents(ctx, smReq)
	if err != nil {
		return nil, err
	}

	return &cloudapipb.GetLiveViewContentsResp{
		Metadata: &cloudapipb.LiveViewMetadata{
			ID:   req.LiveViewID,
			Name: smResp.Metadata.Name,
			Desc: smResp.Metadata.Desc,
		},
		PxlContents: smResp.PxlContents,
		Vis:         smResp.Vis,
	}, nil
}

// GetScripts returns a list of all available scripts.
func (s *ScriptMgrServer) GetScripts(ctx context.Context, req *cloudapipb.GetScriptsReq) (*cloudapipb.GetScriptsResp, error) {
	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}

	smReq := &scriptmgrpb.GetScriptsReq{}
	smResp, err := s.ScriptMgr.GetScripts(ctx, smReq)
	if err != nil {
		return nil, err
	}
	resp := &cloudapipb.GetScriptsResp{
		Scripts: make([]*cloudapipb.ScriptMetadata, len(smResp.Scripts)),
	}
	for i, script := range smResp.Scripts {
		resp.Scripts[i] = &cloudapipb.ScriptMetadata{
			ID:          pbutils.UUIDFromProtoOrNil(script.ID).String(),
			Name:        script.Name,
			Desc:        script.Desc,
			HasLiveView: script.HasLiveView,
		}
	}
	return resp, nil
}

// GetScriptContents returns the pxl string of the script.
func (s *ScriptMgrServer) GetScriptContents(ctx context.Context, req *cloudapipb.GetScriptContentsReq) (*cloudapipb.GetScriptContentsResp, error) {
	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}

	smReq := &scriptmgrpb.GetScriptContentsReq{
		ScriptID: pbutils.ProtoFromUUIDStrOrNil(req.ScriptID),
	}
	smResp, err := s.ScriptMgr.GetScriptContents(ctx, smReq)
	if err != nil {
		return nil, err
	}
	return &cloudapipb.GetScriptContentsResp{
		Metadata: &cloudapipb.ScriptMetadata{
			ID:          req.ScriptID,
			Name:        smResp.Metadata.Name,
			Desc:        smResp.Metadata.Desc,
			HasLiveView: smResp.Metadata.HasLiveView,
		},
		Contents: smResp.Contents,
	}, nil
}

// ProfileServer provides info about users and orgs.
type ProfileServer struct {
	ProfileServiceClient profilepb.ProfileServiceClient
}

// GetOrgInfo gets the org info for a given org ID.
func (p *ProfileServer) GetOrgInfo(ctx context.Context, req *uuidpb.UUID) (*cloudapipb.OrgInfo, error) {
	ctx, err := contextWithAuthToken(ctx)
	if err != nil {
		return nil, err
	}

	sCtx, err := authcontext.FromContext(ctx)
	if err != nil {
		return nil, err
	}
	claimsOrgID := sCtx.Claims.GetUserClaims().OrgID
	orgID := pbutils.UUIDFromProtoOrNil(req)
	if claimsOrgID != orgID.String() {
		return nil, status.Error(codes.Unauthenticated, "Unable to fetch org info")
	}

	resp, err := p.ProfileServiceClient.GetOrg(ctx, req)
	if err != nil {
		return nil, err
	}

	return &cloudapipb.OrgInfo{
		ID:      resp.ID,
		OrgName: resp.OrgName,
	}, nil
}
